#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/params/Params.h"
#include "cinder/audio/audio.h"
#include "cinder/Easing.h"

#include "Band.h"
#include "Range.h"
#include "File.h"

#define	 MIC                1       // microphone: 0 to play file.mp3 / 1 for microphone
#define	 LOGBANDS           11      // number of subbands for log bands repartition
#define	 BUFFERSIZE         43      // arround 1s storage at 44100
#define	 SPECTRUMBANDS      8       // number of subbands for spectrun range magnitude calculation
#define	 MAXSPECTRUMBINS    128     // number max of bins for spectrun range magnitude calculation


using namespace ci;
using namespace ci::app;
using namespace std;

class AudioBeatAnalyzerApp : public App {
private:
    enum : int32_t
    {
        Beat0,
        Beat1,
        Beat2
    } typedef EBeat;
    
    enum : int32_t
    {
        Energy,
        EaseExpo,
        Smoothed
    } typedef EMode;
    
public:
    static void prepareSettings( Settings *settings );
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void update() override;
    void draw() override;
    
    void initAudio();
    void initLogBands( int nbBands );
    void initSpectrumRange( int nbBands );
    void computeLogSoundEnergy( const vector<float> &magSpectrum );
    void computeSoundRange( const vector<float> &magSpectrum );
    void computeRangeEnergy( const vector<float> &magSpectrum );
    void drawLogSoundEnergy( const vector<float> &magSpectrum );
    void drawLogSoundBeat();
    void drawMagnitudes();
    void drawLogBandBeatTreshold();
    void drawSpectrum();
    double computeAverrage( list<double> &historyBuffer );
    double computeVariance( list<double> &historyBuffer, double averageEnergy );
    void recordAudioInit( File &file );
    void recordAudio( File &file );
    
    // AUDIO
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNodeBand;       // SpectralNode for log Bands
    audio::MonitorSpectralNodeRef	mMonitorSpectralNodeSpectrum;   // SpectralNode for spectrum
    
    audio::GainNodeRef				mGainNodeBand;
    audio::GainNodeRef				mGainNodeSpectrum;
    audio::BufferPlayerNodeRef		mBufferPlayerNode;
    audio::FilterHighPassNodeRef	mHighPass;                      // Lowpass filter to reduce low frequency content
    audio::FilterBiquadNode         *mLowShelf = NULL;
    float                           mLowShelfFrequency = 0.0f;
    float                           mLowShelfGain = 0.0f;
    
    vector<float>					mMagSpectrumLog;                // Spectrum for log Bands
    vector<float>					mMagSpectrumSpectrum;           // Spectrum for spectrum
    vector<Band>                    mBands;                         // Bands for log Bands
    vector<int>                     mSpectrumBandSizes;             // Bands for log spectrum
    Range                           mRange;                         // Range for spectrum Magnitude
    
    // DRAWING //////////////////////////////////////////////////
    gl::BatchRef                    mBatchRect;
    gl::VertBatchRef                mBatchLines;
    vector<float>                   mMagnitudes;
    bool                            mblink = false;
    float                           mBlinkCounter = 0.0f;
    
    // PARAM ////////////////////////////////////////////////////
    params::InterfaceGlRef          mParams;
    float                           mFrameRate = 0.0f;
    float                           mBandGain = 200.0f;
    float                           mSpectrumGain = 400.0f;
    double                          mBandSmoothingFactor = 0.0;         // smooth in fft for bands energy
    double                          mBandSmoothing = 0.8;               // smooth bands Energy after fft, better to set at 1.0 if mBandSmoothingFactor < 1.0 to not smooth 2 times
    double                          mSpectrumSmoothingFactor = 0.7;     // smooth in fft for spectrum energy
    
    // Beat and magnitude calculation
    int                             mBand0 = 1;
    int                             mBand1 = 4;
    int                             mBand2 = 9;
    double                          mBeatSensitivityCoef[3] = { 1.0, 1.0, 1.0 };
    double                          mBeatThreshold[3] = { 0.05, 0.05, 0.02 };
    double                          mMagnDecay[4] = { 0.90, 0.90, 0.90, 0.90 };
    double                          mMagnitudeMin[3] = { 0.0, 0.0, 0.0 };
    double                          mMagnitudeMax[3] = { 1.0, 1.0, 1.0 };
    bool                            mSmoothing[4] = { true, true, true, true };
    double                          mMagnitudeOutCoef[4] = { 1.0, 1.0, 1.0, 1.0 };
    
    // Beat and Magnitude Selection
    int32_t                         mBeat = Beat0;
    int32_t                         mMagnitudeOutMode[4] = { Energy, Energy, Energy, Smoothed };
    bool                            mActivateBeat0 = false;
    bool                            mActivateBeat1 = false;
    bool                            mActivateBeat2 = false;
    
    
    // OFSTREAM //////////////////////////////////////////////////
    File                            mTxtFile;
    

    // COLORS ////////////////////////////////////////////////////
    ColorA                          yellow = vec4( 1.0f, 1.0f, 0.60f, 1.0f );
    ColorA                          rosa = vec4( 1.0f, 0.76f, 0.70f, 1.0f );
    ColorA                          cyan = vec4( 0.39f, 0.92f, 1.00f, 1.0f );
    ColorA                          green = vec4( 0.78f, 0.89f, 0.62f, 1.0f );
    ColorA                          overSizeColor = vec4( 1.0f, 0.0f, 0.0f, 1 );
    ColorA                          grey = vec4( 0.5f, 0.5f, 0.5f, 1.0f );
};


void AudioBeatAnalyzerApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 60 );
    settings->setWindowSize( 1280, 720 );
}


void AudioBeatAnalyzerApp::setup()
{
    initLogBands( LOGBANDS );
    initSpectrumRange( SPECTRUMBANDS );
    initAudio();
    
    // 3 Log Band magnitudes and 1 spectrum Range magnitude
    mMagnitudes.push_back(0.0f);
    mMagnitudes.push_back(0.0f);
    mMagnitudes.push_back(0.0f);
    mMagnitudes.push_back(0.0f);
    
    const vector<string> beat = { "Beat0", "Beat1", "Beat2" };
    const vector<string> magnitudeOutCalculationMode = { "Energy", "EaseExpo", "Smoothed" };
    
    // Params
    mParams = params::InterfaceGl::create( getWindow(), "settings", toPixels( ivec2( 230, 220 ) ) );
    mParams->addSeparator();
    mParams->addParam( "Frame rate", &mFrameRate, "group=`Input`", true );
    mParams->addParam( "LowShelf Frequency", &mLowShelfFrequency, "min=-1.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Input`" );
    mParams->addParam( "LowShelf Gain", &mLowShelfGain, "min=-50.0 max=50.0 step=1.0 keyIncr=q keyDecr=w group=`Input`" );
    mParams->addParam( "Band Gain", &mBandGain, "min=0.0 max=1000.0 step=0.5 keyIncr=q keyDecr=w group=`Input`" );
    mParams->addParam( "Bands Smoothing Factor", &mBandSmoothingFactor, "min=0.0 max=1.0 step=0.1 keyIncr=q keyDecr=w group=`Input`" );
    mParams->addParam( "Smoothing Energy", &mBandSmoothing, "min=0.0 max=1.0 step=0.1 keyIncr=q keyDecr=w group=`Input`" );
    mParams->addParam( "Spectrum Smoothing Factor", &mSpectrumSmoothingFactor, "min=0.0 max=1.0 step=0.1 keyIncr=q keyDecr=w group=`Input`" );
    
    mParams->addSeparator();
    mParams->addParam( "0 - Band", &mBand0, "min=0 max=10 step=1 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - Magnitute Out Coef", &mMagnitudeOutCoef[0], "min=0.0 max=10.0 step=0.1 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - BeatSensitivity Coef", &mBeatSensitivityCoef[0], "min=0.0 max=1000.0 step=0.1 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - BeatThreshold", &mBeatThreshold[0], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - Magn Decay", &mMagnDecay[0], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - Magnitude Min", &mMagnitudeMin[0], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - Magnitude Max", &mMagnitudeMax[0], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 0`" );
    mParams->addParam( "0 - Magnitude Out Mode", magnitudeOutCalculationMode, &mMagnitudeOutMode[0], "keyDecr=a keyIncr=b group=`Band 0`" );
    
    mParams->addSeparator();
    mParams->addParam( "1 - Band", &mBand1, "min=0 max=10 step=1 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - Magnitute Out Coef", &mMagnitudeOutCoef[1], "min=0.0 max=10.0 step=0.1 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - BeatSensitivity Coef", &mBeatSensitivityCoef[1], "min=0.0 max=1000.0 step=0.1 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - BeatThreshold", &mBeatThreshold[1], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - Magn Decay", &mMagnDecay[1], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - Magnitude Min", &mMagnitudeMin[1], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - Magnitude Max", &mMagnitudeMax[1], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 1`" );
    mParams->addParam( "1 - Magnitude Out Mode", magnitudeOutCalculationMode, &mMagnitudeOutMode[1], "keyDecr=a keyIncr=b group=`Band 1`" );
    
    mParams->addSeparator();
    mParams->addParam( "2 - Band", &mBand2, "min=0 max=10 step=1 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - Magnitute Out Coef", &mMagnitudeOutCoef[2], "min=0.0 max=10.0 step=0.1 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - BeatSensitivity Coef", &mBeatSensitivityCoef[2], "min=0.0 max=1000.0 step=0.1 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - BeatThreshold", &mBeatThreshold[2], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - Magn Decay", &mMagnDecay[2], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - Magnitude Min", &mMagnitudeMin[2], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - Magnitude Max", &mMagnitudeMax[2], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Band 2`" );
    mParams->addParam( "2 - Magnitude Out Mode", magnitudeOutCalculationMode, &mMagnitudeOutMode[2], "keyDecr=a keyIncr=b group=`Band 2`" );
    
    mParams->addSeparator();
    mParams->addParam( "Spectrum Gain", &mSpectrumGain, "min=0.0 max=1000.0 step=0.5 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "Position", &mRange.position, "min=1 max=128 step=1 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "Range Width", &mRange.rangeWidth, "min=1 max=128 step=1 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - Magnitute Out Coef", &mMagnitudeOutCoef[3], "min=0.0 max=10.0 step=0.1 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - BeatSensitivity Coef", &mRange.sensitivityCoef, "min=0.0 max=1000.0 step=0.1 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - BeatThreshold", &mRange.beatThreshold, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - Magn Decay", &mMagnDecay[3], "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - Magnitude Min", &mRange.magnitudeMin, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - Magnitude Max", &mRange.magnitudeMax, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w group=`Spectrum Range`" );
    mParams->addParam( "3 - Magnitude Out Mode", magnitudeOutCalculationMode, &mMagnitudeOutMode[3], "keyDecr=a keyIncr=b group=`Spectrum Range`" );
    
    mParams->addSeparator();
    mParams->addParam( "Beat", beat, &mBeat, "keyDecr=a keyIncr=b group=`Beat & EnergySelection`" );
    mParams->addParam( "Activate beat 0", &mActivateBeat0 ).key( "0" ).group( "Beat & EnergySelection" );
    mParams->addParam( "Activate beat 1", &mActivateBeat1 ).key( "1" ).group( "Beat & EnergySelection" );
    mParams->addParam( "Activate beat 2", &mActivateBeat2 ).key( "2" ).group( "Beat & EnergySelection" );
    
    // Init drawind
    auto color = gl::ShaderDef().color();
    gl::GlslProgRef shader = gl::getStockShader( color );
    mBatchRect = gl::Batch::create( geom::Rect(), shader );
    mBatchLines = gl::VertBatch::create( GL_LINES );
}


void AudioBeatAnalyzerApp::mouseDown( MouseEvent event )
{
    
}


void AudioBeatAnalyzerApp::keyDown( KeyEvent event )
{
    if ( event.getChar() == 'f' || event.getChar() == 'F' )
    {
        setFullScreen( ! isFullScreen() );
    }
    
    // Play
    if ( event.getChar() == 'p' && !MIC )
    {
        // To start at x seconds
        mBufferPlayerNode->seekToTime( 1540.0 ); // in seconds
        mBufferPlayerNode->start();
    }
    
    // Stop and Stop Recording
    if ( event.getChar() == 's' )
    {
        mTxtFile.closeStream();
        if( !MIC ) mBufferPlayerNode->stop();
    }
    
    // Play and Record
    if ( event.getChar() == 'r' )
    {
        // Verify if the stream is already open
        if ( mTxtFile.isTxtStreamOpen() == true )
            return;
        
        recordAudioInit( mTxtFile );
    }
}


void AudioBeatAnalyzerApp::update()
{
    // We copy the magnitude spectrum out from the Node on the main thread, once per update:
    mMagSpectrumLog = mMonitorSpectralNodeBand->getMagSpectrum();
    mMagSpectrumSpectrum = mMonitorSpectralNodeSpectrum->getMagSpectrum();
    
    computeLogSoundEnergy( mMagSpectrumLog );
    computeSoundRange( mMagSpectrumLog );
    
    // Update Gain on Input
    mGainNodeBand->setValue(mBandGain);
    mGainNodeSpectrum->setValue(mSpectrumGain);
    
    // Update smoothing factors
    mMonitorSpectralNodeBand->setSmoothingFactor( mBandSmoothingFactor ); // smotth for bands
    mMonitorSpectralNodeSpectrum->setSmoothingFactor( mSpectrumSmoothingFactor ); // Smooth the spectrum
    
    // Update shelf
    mLowShelf->setFreq( mLowShelfFrequency );
    mLowShelf->setGain( mLowShelfGain );
    
    // Update Bands
    mBands[mBand0].beatThreshold = mBeatThreshold[0];
    mBands[mBand1].beatThreshold = mBeatThreshold[1];
    mBands[mBand2].beatThreshold = mBeatThreshold[2];
    
    // Other Parameters
    mFrameRate	= getAverageFps();
    mBlinkCounter = cos( 2.0f * getElapsedSeconds() );
    if ( mBlinkCounter < 0 ) mblink = true;
    else mblink = false;
    
    // Ofstream
    recordAudio( mTxtFile );
}


void AudioBeatAnalyzerApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    drawLogSoundEnergy( mMagSpectrumLog );
    drawLogSoundBeat();
    drawMagnitudes();
    drawLogBandBeatTreshold();
    drawSpectrum();
    mParams->draw();
}


void AudioBeatAnalyzerApp::initAudio()
{
    auto ctx = audio::Context::master();
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat1 = audio::MonitorSpectralNode::Format().fftSize( 2048 ).windowSize( 1024 );
    mMonitorSpectralNodeBand = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat1 ) );
    mMonitorSpectralNodeBand->setSmoothingFactor( mBandSmoothingFactor ); // smotth for bands
    
    auto monitorFormat2 = audio::MonitorSpectralNode::Format().fftSize( 1024 ).windowSize( 512 );
    mMonitorSpectralNodeSpectrum = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat2 ) );
    mMonitorSpectralNodeSpectrum->setSmoothingFactor( mSpectrumSmoothingFactor ); // Smooth the spectrum
    
    console() << "ctx->getFftSize(): " << mMonitorSpectralNodeBand->getFftSize() << endl; // should be 2048
    console() << "ctx->getSampleRate(): " << ctx->getSampleRate() << endl; // should be 44100
    console() << "SmoothingFactor for Bands: " << mMonitorSpectralNodeBand->getSmoothingFactor() << endl; // should be 0
    
    // Create channel router nodes and routes: (does not work now)
    // Set the ChannelRouterNode to use only 1 channel
    //auto format = audio::Node::Format().channels(2);
    //auto channelRouter1 = ctx->makeNode(new audio::ChannelRouterNode(format));
    //mBufferPlayerNode >> channelRouter1->route(0, 0) >> mGain >> mMonitorSpectralNode;
    
    
    // Add a Gain to reduce the volume
    mGainNodeBand = ctx->makeNode( new audio::GainNode( mBandGain ) );
    mGainNodeSpectrum = ctx->makeNode( new audio::GainNode( mSpectrumGain ) );
    
    // Shelf exemple, not used now
    mLowShelf = new audio::FilterBiquadNode;
    mLowShelf->setMode( audio::FilterBiquadNode::Mode::LOWSHELF );
    mLowShelf->setGain( mLowShelfGain );
    mLowShelf->setFreq( mLowShelfFrequency );
    auto lowShelf = ctx->makeNode( mLowShelf );
    
    // Not mic
    if ( !MIC )
    {
        // create a SourceFile and set its output samplerate to match the Context.
        audio::SourceFileRef sourceFile = audio::load( loadAsset( "Sorbeats-Herbstbeginn.mp3" ), ctx->getSampleRate() );
        
        // load the entire sound file into a BufferRef, and construct a BufferPlayerNode with this.
        audio::BufferRef buffer = sourceFile->loadBuffer();
        
        mBufferPlayerNode = ctx->makeNode( new audio::BufferPlayerNode( buffer ) );
        
        mBufferPlayerNode >> mGainNodeBand >> mMonitorSpectralNodeBand;
        mBufferPlayerNode >> mGainNodeSpectrum >> mMonitorSpectralNodeSpectrum;
        
        // connect and enable the Context
        mBufferPlayerNode >> ctx->getOutput();
    }
    else{
        
        // The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
        mInputDeviceNode = ctx->createInputDeviceNode();
        
        // Filter exemple, not used now
        //mHighPass = ctx->makeNode( new audio::FilterHighPassNode );
        //mHighPass->setFreq( 1000 );
        //mInputDeviceNode >> mHighPass >> mMonitorSpectralNode;
        
        mInputDeviceNode >> lowShelf >> mGainNodeBand >> mMonitorSpectralNodeBand;
        mInputDeviceNode >> lowShelf >> mGainNodeSpectrum >> mMonitorSpectralNodeSpectrum;
        
        // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
        mInputDeviceNode->enable();
    }
    
    ctx->enable();
}


// Band with logarithmic repartion ( or Octave repartion )
/*
 Octaves represent the overall level of energy over a specific frequency range.
 In a logarithmic repartition ( by octave ) each octave having double frequency span of the previous octave
 ( https://community.sw.siemens.com/s/article/octaves-in-human-hearing )
 
 The first audible band goes from 11 to 22 Hz -> 11 Hz band
 The second audible band goes from 22 to 44 Hz -> 22 Hz band
 The third audible band goes from 44 to 48 Hz -> 22 Hz band
 ..
 The last audible band goes from 11360 to 22720 Hz -> 11360 Hz band
 
 So we have 11 audible bands
 
 For samplerate of 44100 Hz and window of 1024 samples lineary distributed, we can calculate it:
 mMonitorSpectralNode->getFreqForBin( 1 ) - mMonitorSpectralNode->getFreqForBin( 0 ) = 21.5332 Hz
 
 We will create bands by doubling the number of Bins per band
 
 We do not take the first band between 0 a 11 Hz because those frequencies are inaudible for human hearing
 We start with the 11 a 22 hz band: for this one we will only have 1 fft output sample : 21.5332 Hz
 
 Theoretically the 22 a 44 hz band should contains 2 samples but with the configured fft parameters we will still have only 1 sample: 43.0664 Hz
 
 Then the number of of output samples will effectly double for each band
*/
void AudioBeatAnalyzerApp::initLogBands( int nbBands )
{
    
    for( int i = 0; i < nbBands; i++ ) {
        mBands.push_back(Band());
    }
    
    for( int i = 0; i < mBands.size(); i++ ) {
        
        mBands[i].active = true;
        mBands[i].beat = false;
        mBands[i].beatCounter = 11;
        mBands[i].beatSensitivity = 0.04; // Default, must be adjusted depends on the number of bin in the Band
        mBands[i].bandEnergy = 0.0;
        mBands[i].bandMagnitude = 0.0;
        mBands[i].smoothBandMagnitude = 0.0;
        mBands[i].bandMagnitudeCorrected = 0.0;
        mBands[i].bandSize = 1;
    }
    
    // Init BUFFERSIZE (43) history Buffer elements
    for ( int i = 0; i < mBands.size(); i++ ) {
        for ( int j = 0; j < BUFFERSIZE; j++ ) {
            mBands[i].historyEnergyBuffer.push_back( 0.0 );
        }
    }
    
    // Compute bandsize for each octave, assuming Samplerate is 44100 and fftsize is 2048;
    float binFreq = 44100.0f / 2048.0f;
    
    // Octave 0 Range is 11 Hz and each octave having double frequency span of the previous octave
    int spanFreq = 11;
    int nextSpanFreq = 0;
    
    for ( int i = 1; i < 12; i++ ) {
        nextSpanFreq = spanFreq * 2;
        if ( nextSpanFreq == 176 ) nextSpanFreq = 177; // see octave chart from https://community.sw.siemens.com/s/article/octaves-in-human-hearing
        if ( nextSpanFreq == 354 ) nextSpanFreq = 355; // see octave chart from https://community.sw.siemens.com/s/article/octaves-in-human-hearing
        //console() << "lower octave freq: " << spanfreq << endl;
        int bandSize = 0;
        for ( int i = 0; i < 1024; i++ )
        {
            if ( binFreq * i > spanFreq && binFreq * i < nextSpanFreq )
            {
                bandSize += 1;
                //console() << "-- bin freq: " << binFreq * i << endl;
            }
        }
        
        mBands[i-1].bandSize = bandSize;
        mBands[i-1].beatSensitivity = 0.02/(double)bandSize;
        console() << " -- bandzise: " << bandSize << endl;
        //console() << "higher octave freq: " << nextSpanfreq << endl;
        spanFreq = nextSpanFreq;
    }
}


void AudioBeatAnalyzerApp::initSpectrumRange( int nbBands )
{
    for( int i = 0; i < nbBands; i++ ) {
        mSpectrumBandSizes.push_back(1);
    }
    
    /*
     Here we don't want the beat but the sound energy. We only take human hearing frequencies without ultrass and harmonics
     From the 512 bins we take from 1 ( 43.06 hz ) - > 132 ( 5684.7 hz)
     Init band Size starting at band 2 (because the 2 firt bands have 1 bin only)
     bin 0 is 0 -> we don't need it
     band 1 bin 1        : 1 bin
     band 2 bin 2        : 1 bin
     band 3 bin 3 - 4    : 2 bins
     band 4 bin 5 - 8    : 4 bins
     band 5 bin 9 - 16   : 8 bins
     band 6 bin 17 - 32  : 16 bins
     band 7 bin 33 -  65 : 32 bins
     band 8 bin 66 -  132 : 64 bins
    */
    for ( int i = 2; i < mSpectrumBandSizes.size(); i++ )
    {
        mSpectrumBandSizes[i] = mBands[i-1].bandSize * 2;
        console() << "band " << i << "bandSize: " << mSpectrumBandSizes[i] << endl;
    }
    
    // Init Spectrum Range
    mRange.active = true;
    mRange.beat = false;
    mRange.beatCounter = 11;
    mRange.beatSensitivity = 0.04;
    mRange.sensitivityCoef = 1.0;
    mRange.beatThreshold = 0.0;
    mRange.rangeEnergy = 0.0;
    mRange.rangeMagnitude = 0.0;
    mRange.rangeMagnitudeCorrected = 0.0;
    mRange.smoothRangeMagnitude= 0.0;
    mRange.magnitudeMin = 0.0;
    mRange.magnitudeMax = 1.0;
    mRange.magnDecay = 0.90;
    mRange.position = 3;
    mRange.rangeWidth = 7;
    
    // Init BUFFERSIZE (43) history Buffer elements
    for ( int j = 0; j < BUFFERSIZE; j++ ) {
        mRange.historyEnergyBuffer.push_back( 0.0 );
    }
}


void AudioBeatAnalyzerApp::computeLogSoundEnergy( const vector<float> &magSpectrum )
{
    if( magSpectrum.empty() )
        return;
    
    // First bin not used because we don't hear it
    //console() << " Bin 0 -> freq: " << mMonitorSpectralNodeBand->getFreqForBin( 0 ) << " magSpectrum " << magSpectrum[0] << endl;
    
    // For bands nb > 0
    int binPosition = 1;
    for ( int i = 0; i < 11; i++ )
    {
        //console() << " firstBinInBand " << firstBinInBand << endl;
        //console() << " binsPerBand " << binsPerBand << endl;
        
        double bandEnergy = 0.0;
        int bandSize = mBands[i].bandSize;
        
        for ( int j = 0; j < bandSize; j++ )
        {
            // Compute average band Energy
            bandEnergy += magSpectrum[ binPosition + j ];
            //console() << " Band " << i << " Bin " << binPosition + j << " -> freq: " << mMonitorSpectralNodeBand->getFreqForBin( binPosition + j )<< endl;
        }
        
        bandEnergy = bandEnergy / bandSize;
        binPosition += bandSize;
        
        // Compute average band Energy
        double averageEnergy = computeAverrage( mBands[i].historyEnergyBuffer );
        
        // Shift Energy History Buffer
        mBands[i].historyEnergyBuffer.pop_back();
        mBands[i].historyEnergyBuffer.push_front( bandEnergy );
        mBands[i].beat = false;
        
        // Beat calculation
        double variance = computeVariance( mBands[i].historyEnergyBuffer, averageEnergy );
        double c = -15.0 * variance + 1.55;
        double  sensibilityCoef = 1.0;
        double  beatThreshold = 10.0; // Choose a default value that will always be higher than the energy
        double  decay = 1.0;
        double  magnitudeMin = 0.0;
        double  magnitudeMax = 0.0;
        double  magnitudecoef = 1.0;
        double  smoothed = false;
        int32_t magnitudeOutCalculationMode = Energy;
        
        if ( i == mBand0 ) { sensibilityCoef = mBeatSensitivityCoef[0]; beatThreshold = mBeatThreshold[0]; magnitudecoef = mMagnitudeOutCoef[0];
            decay = mMagnDecay[0]; magnitudeMin = mMagnitudeMin[0];  magnitudeMax = mMagnitudeMax[0]; smoothed = mSmoothing[0]; magnitudeOutCalculationMode = mMagnitudeOutMode[0]; }
        if ( i == mBand1 ) { sensibilityCoef = mBeatSensitivityCoef[1]; beatThreshold = mBeatThreshold[1]; magnitudecoef = mMagnitudeOutCoef[1];
            decay = mMagnDecay[1]; magnitudeMin = mMagnitudeMin[1];  magnitudeMax = mMagnitudeMax[1]; smoothed = mSmoothing[1]; magnitudeOutCalculationMode = mMagnitudeOutMode[1]; }
        if ( i == mBand2 ) { sensibilityCoef = mBeatSensitivityCoef[2]; beatThreshold = mBeatThreshold[2]; magnitudecoef = mMagnitudeOutCoef[2];
            decay = mMagnDecay[2]; magnitudeMin = mMagnitudeMin[2];  magnitudeMax = mMagnitudeMax[2]; smoothed = mSmoothing[2]; magnitudeOutCalculationMode = mMagnitudeOutMode[2]; }
        
        // Only calculate beat and magnitudes for selected bands
        if ( i == mBand0 || i == mBand1 || i == mBand2 )
        {
            mBands[i].smoothBandMagnitude = mBands[i].smoothBandMagnitude * mBandSmoothing + bandEnergy * ( 1.0 - mBandSmoothing );
            
            // Beat calculation
            if ( bandEnergy - ( sensibilityCoef * mBands[i].beatSensitivity ) > c * averageEnergy && bandEnergy > beatThreshold && mBands[i].beatCounter > 10 )
            {
                mBands[i].beat = true;
                mBands[i].beatCounter = 0;
                mBands[i].bandEnergy = bandEnergy;
                if ( magnitudeOutCalculationMode != EaseExpo ) mBands[i].bandMagnitude = bandEnergy;
            }
            else if ( mBands[i].beatCounter < 11 && magnitudeOutCalculationMode == EaseExpo )
            {
                mBands[i].bandMagnitude += easeOutExpo( mBands[i].beatCounter ) * mBands[i].bandEnergy;
            }
            
            // Minimum delay between 2 beats: 16.67 ms at framerate = 60 => 3600 bpm max for beat detection
            if ( mBands[i].beatCounter < 11 )
                mBands[i].beatCounter += 1;
            
            
            mBands[i].bandMagnitude *= decay;
            mBands[i].bandMagnitude = clamp( mBands[i].bandMagnitude, magnitudeMin, magnitudeMax);
            
            if ( magnitudeOutCalculationMode == Smoothed )
            {
                mBands[i].smoothBandMagnitude *= decay;
                mBands[i].smoothBandMagnitude = clamp( mBands[i].smoothBandMagnitude, magnitudeMin, magnitudeMax);
                mBands[i].bandMagnitudeCorrected = mBands[i].smoothBandMagnitude * magnitudecoef;
            }
            else mBands[i].bandMagnitudeCorrected = mBands[i].bandMagnitude * magnitudecoef;
        }
    }
}


void AudioBeatAnalyzerApp::computeSoundRange( const vector<float> &magSpectrum )
{
    int maxRange = MAXSPECTRUMBINS - mRange.rangeWidth - mRange.position;
    //console() << ">> maxRange " << maxRange << endl;
    
    if ( mRange.position > 127 )
        mRange.position = 127;
    
    if ( mRange.position < 0 )
        mRange.position = 0;
    
    // maxRange is < 0 so make + to substract
    if ( maxRange <= 0 )
        mRange.rangeWidth = mRange.rangeWidth + maxRange;
    
    mRange.beatSensitivity = 0.02/(double)mRange.rangeWidth;
    computeRangeEnergy ( magSpectrum );
}


void AudioBeatAnalyzerApp::computeRangeEnergy( const vector<float> &magSpectrum )
{
    if( mMagSpectrumSpectrum.empty() )
        return;
    
    double rangeEnergy = 0.0;
    
    for( size_t i = 0; i < mRange.rangeWidth; i++ )
    {
        rangeEnergy += mMagSpectrumSpectrum[ mRange.position + i ]  ;
    }
    
    rangeEnergy /= (double)mRange.rangeWidth;
    //console() << ">> bandEnergy " << bandEnergy << endl;
    
    // Compute average range Energy
    double averageEnergy = computeAverrage( mRange.historyEnergyBuffer );
    
    // Shift Energy History Buffer
    mRange.historyEnergyBuffer.pop_back();
    mRange.historyEnergyBuffer.push_front( rangeEnergy );
    mRange.beat = false;
    
    // Beat calculation
    double variance = computeVariance( mRange.historyEnergyBuffer, averageEnergy );
    double c = ( -0.025714 * variance ) + 1.5142857;
    
    mRange.smoothRangeMagnitude = rangeEnergy;
    
    if ( rangeEnergy > c * averageEnergy && rangeEnergy > mRange.beatThreshold && mRange.beatCounter > 10 )
    {
        mRange.beat = true;
        mRange.beatCounter = 0;
        mRange.rangeEnergy = rangeEnergy;
        if ( mMagnitudeOutMode[3] != EaseExpo ) mRange.rangeMagnitude = rangeEnergy;
    }
    else if ( mRange.beatCounter < 1.0f && mMagnitudeOutMode[3] == EaseExpo )
    {
        mRange.rangeMagnitude += easeOutExpo( mRange.beatCounter ) * mRange.rangeEnergy;
    }
    
    // Minimum delay between 2 beats: 16.67 ms at framerate = 60 => 3600 bpm max for beat detection
    if ( mRange.beatCounter < 11 )
        mRange.beatCounter += 1;
    
    mRange.rangeMagnitude *= mMagnDecay[3];
    mRange.rangeMagnitude = clamp( mRange.rangeMagnitude, mRange.magnitudeMin, mRange.magnitudeMax );
    
    if ( mMagnitudeOutMode[3] == Smoothed )
    {
        mRange.smoothRangeMagnitude *= mMagnDecay[3];
        mRange.smoothRangeMagnitude = clamp( mRange.smoothRangeMagnitude, mRange.magnitudeMin, mRange.magnitudeMax);
        mRange.rangeMagnitudeCorrected = mRange.smoothRangeMagnitude * mMagnitudeOutCoef[3];
    }
    else mRange.rangeMagnitudeCorrected = mRange.rangeMagnitude * mMagnitudeOutCoef[3];
}


void AudioBeatAnalyzerApp::drawLogSoundEnergy( const vector<float> &magSpectrum )
{
    float shiftBottom = 255.0f;
    float shiftRight = 20.0f;
    float padding = 15.0f;
    float bandWidth = 10.0;
    int bandNumber = mBands.size();
    
    for( size_t i = 0; i < bandNumber; i++ )
    {
        float g = (float)i / ( float)bandNumber;
        ColorA beatColor( 0.5f, g, 1.0f, 1 );
        float bandHeight = (float)mBands[i].historyEnergyBuffer.front() * 100.0f;
        
        if ( bandHeight >= 100.0f )
        {
            gl::color( overSizeColor );
            bandHeight = 100.0f;
        }
        else if ( mBands[i].beat == true ) gl::color( beatColor );
        else gl::color( grey );
        
        gl::pushModelMatrix();
        gl::translate( vec3( shiftRight + padding * i, shiftBottom - bandHeight / 2.0f, 0.0f ) );
        gl::scale( vec3( bandWidth, bandHeight, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
}


void AudioBeatAnalyzerApp::drawLogSoundBeat()
{
    gl::ScopedGlslProg glslScope( getStockShader( gl::ShaderDef().color() ) );
    
    float shiftBottom = 265.0f;
    float shiftRight = 20.0f;
    float padding = 15.0f;
    float bandWidth = 10.0f;
    
    bool beat0 = mBands[mBand0].beat;
    bool beat1 = mBands[mBand1].beat;
    bool beat2 = mBands[mBand2].beat;
    
    if( beat0 == true ){
        gl::color( cyan );
        gl::pushModelMatrix();
        gl::translate( vec3( shiftRight + padding * mBand0, shiftBottom, 0.0f ) );
        gl::scale( vec3( bandWidth, bandWidth, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
    if( beat1 == true ){
        gl::color( rosa);
        gl::pushModelMatrix();
        gl::translate( vec3( shiftRight + padding * mBand1, shiftBottom, 0.0f ) );
        gl::scale( vec3( bandWidth, bandWidth, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
    if( beat2 == true ){
        gl::color( green );
        gl::pushModelMatrix();
        gl::translate( vec3( shiftRight + padding * mBand2, shiftBottom, 0.0f ) );
        gl::scale( vec3( bandWidth, bandWidth, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
}


void AudioBeatAnalyzerApp::drawMagnitudes()
{
    gl::ScopedGlslProg glslScope( getStockShader( gl::ShaderDef().color() ) );
    
    mMagnitudes[0] = (float)mBands[mBand0].bandMagnitudeCorrected;
    mMagnitudes[1] = (float)mBands[mBand1].bandMagnitudeCorrected;
    mMagnitudes[2] = (float)mBands[mBand2].bandMagnitudeCorrected;
    mMagnitudes[3] = (float)mRange.rangeMagnitudeCorrected;
    
    
    float shiftBottom = 255.0f;
    float shiftRight = 220.0f;
    float padding = 15.0f;
    float bandWidth = 10.0f;
    int bandNumber = mMagnitudes.size();
    
    for( size_t i = 0; i < bandNumber; i++ )
    {
        gl::color( cyan );
        if ( i == 1 ) gl::color( rosa );
        else if ( i == 2 ) gl::color( green );
        else if ( i == 3 ) gl::color( yellow );
        
        float g = (float)i / ( float)bandNumber;
        ColorA beatColor( 0.5f, g, 1.0f, 1 );
        
        float bandHeight = mMagnitudes[i] * 100.0f;
        if ( bandHeight >= 100.0f )
        {
            gl::color( overSizeColor );
            bandHeight = 100.0f;
        }
        
        gl::pushModelMatrix();
        gl::translate( vec3( shiftRight + padding * i, shiftBottom - bandHeight / 2.0f, 0.0f ) );
        gl::scale( vec3( bandWidth, bandHeight, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
}


void AudioBeatAnalyzerApp::drawLogBandBeatTreshold()
{
    gl::ScopedGlslProg glslScope( getStockShader( gl::ShaderDef().color() ) );
    
    float shiftBottom = 255.0f;
    float shiftRight = 20.0f;
    float padding = 15.0f;
    float bandWidth = 10.0f;
    
    float bandHeight = (float)mBands[mBand0].beatThreshold * 100.0f;
    gl::color( cyan );
    gl::pushModelMatrix();
    gl::translate( vec3( shiftRight + padding * mBand0, shiftBottom - bandHeight, 0.0f ) );
    gl::scale( vec3( bandWidth, 1.0f, 0.0f ) );
    mBatchRect->draw();
    gl::popModelMatrix();
    
    bandHeight = (float)mBands[mBand1].beatThreshold * 100.0f;
    gl::color( rosa );
    gl::pushModelMatrix();
    gl::translate( vec3( shiftRight + padding * mBand1, shiftBottom - bandHeight, 0.0f ) );
    gl::scale( vec3( bandWidth, 1.0f, 0.0f ) );
    mBatchRect->draw();
    gl::popModelMatrix();
    
    bandHeight = (float)mBands[mBand2].beatThreshold * 100.0f;
    gl::color( green );
    gl::pushModelMatrix();
    gl::translate( vec3( shiftRight + padding * mBand2, shiftBottom - bandHeight, 0.0f ) );
    gl::scale( vec3( bandWidth, 1.0f, 0.0f ) );
    mBatchRect->draw();
    gl::popModelMatrix();
}


void AudioBeatAnalyzerApp::drawSpectrum()
{
    mBatchLines = gl::VertBatch::create( GL_LINES );
    
    float shiftBottom = 140.0f;
    float shiftRight = 12.0f;
    float padding = 2.0f;
    
    ColorA color( 0.0, 0.0f, 0.0f, 1.0f );
    
    // Draw Magnitude Range Selection
    for( size_t i = 0; i < mRange.rangeWidth; i++ )
    {
        int xpos = shiftRight + ( mRange.position + i ) * padding;
        float magnitude = (float)mMagSpectrumSpectrum[mRange.position + i] * 100.0f;
        
        // Other drawing position
        if ( magnitude >= 100.0f ) magnitude = 100.0f;
        float lineHeight = shiftBottom - magnitude;
        if ( magnitude >= 30.0f ) magnitude = 30.0f;
        float ypos1 = lineHeight - magnitude;
        color = yellow;
        mBatchLines->color( color );
        mBatchLines->vertex( xpos, lineHeight );
        mBatchLines->color( color );
        mBatchLines->vertex( xpos, ypos1 );
    }
    
    // Here we don't want the beat but the sound energy
    // We only take human hearing frequencies without ultrass and harmonics
    // From the 512 bins we take from 1 ( 43.06 hz ) - > 132 ( 5684.7 hz)
    int nbBands = mSpectrumBandSizes.size();
    int bin = 1;
    
    for( size_t i = 0; i < nbBands; i++ )
    {
        for ( size_t j = 0; j < mSpectrumBandSizes[i]; j++ )
        {
            int xpos = shiftRight + padding * bin;
            float lineHeight = (float)mMagSpectrumSpectrum[bin] * 100.0f;
            
            // We want to give the lines the same color as the 3 log magintude bands
            // logband 0 -> mSpectrumBand 0 up to logband 7 -> mSpectrumBand 7
            if ( i == mBand0 ) { color = cyan; }
            else if ( i == mBand1 ) { color = rosa; }
            else if ( i == mBand2 ) { color = green; }
            else {
                float g = (float)bin / ( float)255;
                ColorA colorBase( 0.5f, g, 1.0f, 1.0f );
                color = colorBase;
            }
            
            if ( lineHeight >= 100.0f )
            {
                color = overSizeColor;
                lineHeight = 100.0f;
            }
            
            mBatchLines->color( color );
            mBatchLines->vertex( xpos, shiftBottom );
            mBatchLines->color( color );
            mBatchLines->vertex( xpos, shiftBottom - lineHeight );
            
            bin +=1;
        }
    }
    mBatchLines->draw();
    
    // Beat
    if( mRange.beat == true )
    {
        float xpos = 0.0f;
        if ( mRange.rangeWidth > 5 ) xpos = shiftRight + ( (float)mRange.position * padding + (float)mRange.rangeWidth * padding / 2.0f);
        else xpos = shiftRight + ( (float)mRange.position * padding) + 5.0f;
            
        gl::color( yellow );
        gl::pushModelMatrix();
        gl::translate( vec3( xpos, shiftBottom + 10.0f, 0.0f ) );
        gl::scale( vec3( 10.0f, 10.0f, 0.0f ) );
        mBatchRect->draw();
        gl::popModelMatrix();
    }
    
    // Beat treshold
    float bandHeight = (float)mRange.beatThreshold * 100.0f;
    float xpos = shiftRight + ( (float)mRange.position * padding + (float)mRange.rangeWidth * padding / 2.0f);
    gl::color( yellow );
    gl::pushModelMatrix();
    gl::translate( vec3( xpos, shiftBottom - bandHeight, 0.0f ) );
    gl::scale( vec3( padding * (float)mRange.rangeWidth, 1.0f, 0.0f ) );
    mBatchRect->draw();
    gl::popModelMatrix();
}


double AudioBeatAnalyzerApp::computeVariance( list<double> &historyBuffer, double averageEnergy )
{
    double v = 0;
    
    for( list<double>::iterator it = historyBuffer.begin(); it != historyBuffer.end(); ++it ){
        v += math<float>::pow( *it - averageEnergy, 2 );
    }
    v /= historyBuffer.size();
    
    return v;
}


double AudioBeatAnalyzerApp::computeAverrage( list<double> &historyBuffer )
{
    double t = 0;
    for( list<double>::iterator it = historyBuffer.begin(); it != historyBuffer.end(); ++it ){
        t += *it;
    }
    return t / historyBuffer.size();
}


// Other init values could be also stored in an init File here before recording audio analysis data
void AudioBeatAnalyzerApp::recordAudioInit( File &file )
{
    // Store init data in an other file
    file.openStream( "assets/output files/audio.txt" );
    
    if( file.isTxtStreamOpen() == true )
    {
        file.setTxtIterator(0);
        file.writeLine( "<?xml version=\"1.0\"?>\n\n" );
        
        // To start at x seconds
        if( !MIC )
        {
            mBufferPlayerNode->seekToTime( 1540.0 ); // Choose the time in seconds if you do not want to start at the beginning
            mBufferPlayerNode->start(); // Start playing
        }
    }
}


// Records audio analysis data
void AudioBeatAnalyzerApp::recordAudio( File &file )
{
    // Only if stream is open
    if( file.isTxtStreamOpen() == true )
    {
        file.incrementTxtIterator();
        file.writeLine( "<" + to_string( file.getTxtIterator() ) + ">\n");
        file.writeLine( "<1>" + to_string( mBands[mBand0].beat ) + "</1>\n");
        file.writeLine( "<2>" + to_string( mBands[mBand0].bandMagnitudeCorrected ) + "</2>\n");
        file.writeLine( "<3>" + to_string( mBands[mBand1].beat ) + "</3>\n");
        file.writeLine( "<4>" + to_string( mBands[mBand1].bandMagnitudeCorrected ) + "</4>\n");
        file.writeLine( "<5>" + to_string( mBands[mBand2].beat ) + "</5>\n");
        file.writeLine( "<6>" + to_string( mBands[mBand2].bandMagnitudeCorrected ) + "</6>\n");
        file.writeLine( "<7>" + to_string( mRange.beat ) + "</7>\n");
        file.writeLine( "<8>" + to_string( mRange.rangeMagnitudeCorrected ) + "</8>\n");
        file.writeLine( "</" + to_string( file.getTxtIterator() ) + ">\n");
    }
}


CINDER_APP( AudioBeatAnalyzerApp, RendererGl , &AudioBeatAnalyzerApp::prepareSettings )
