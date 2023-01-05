
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "Resources.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"
#include "cinder/gl/Texture.h"

//#define SIDE1    800
//#define SIDE2    1280
//#define SIDE1    1080
//#define SIDE2    1920

// 2K
#define SIDE1    2048
#define SIDE2    1080

// 4K
//#define SIDE1    3840
//#define SIDE2    2160


using namespace ci;
using namespace ci::app;
using namespace std;


class DomainWarpingApp : public App {
public:
    static void prepareSettings( Settings *settings );
    void setup() override;
    void update() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void resize() override;
    //void hideCursor() override;
    void draw() override;
    void saveImage();

    
    CameraPersp                 mCam;
    gl::GlslProgRef             mGlsl;
    gl::GlslProgRef             mSNoiseCurlGlsl, mFxaaShader;
    gl::BatchRef                mBatchToScreenRect, mBatchFxaaRect;

    geom::Rect                  mRect;
    
    float                       mCurrentSeconds1, mCurrentSeconds2;
    float                       mDeformationScale = 0.0f;
    float                       mNoiseScale = 180.0f;
    float                       Xdeformation = 0.0f;
    float                       Ydeformation = 0.0f;


    float                       mLength_scale = 0.0f;
    float                       mZoom = 0.0f;
    float                       mZoomValue = -4.0f;
    bool                        mZoomIn = false;
    bool                        mStopZoom = false;
    float                       mShade = 1.0f;
    bool                        mStartShade = false;
    float                       mNoiseScaleLimit = 530.0f;
    float                       mDeformationScaleLimit = 0.0f;
    float                       mXdeformationLimit = 2.0f;
    float                       mYdeformationLimit = 0.0f;

    
    
    float                       mMaxAnisoFilterAmount;
    
    // TEXTURES /////////////////////////////////////////////////
    Surface32f                  perlinTexure1;
    gl::TextureRef              mPosition;
    
    // FBOS /////////////////////////////////////////////////////
    int                         mCurrentFBO, mOtherFBO;
    gl::FboRef                  mRenderFBO;
    ci::gl::FboRef				mFboBufferTest[2];
    ci::gl::Texture2dRef		mTextureFboBuffer[ 2 ], mTextureFboBuffer2[ 2 ], mTexturemRender[ 1 ];
    
    // PARAM ////////////////////////////////////////////////////
    params::InterfaceGlRef      perlinParams;
    float                       mFrameRate = 0.0f;
    
    // SAVE IMAGE ///////////////////////////////////////////////
    int                         mFrameCounter = 0;
    bool                        mSaveScreen = false;
    
    // Other
    float                       mCounter_1 = 0.0f;
    int                         mCycleCounter = 0;
};


void DomainWarpingApp::prepareSettings( Settings *settings )
{

    //settings->setWindowSize( 3840, 2160 );
    settings->setWindowSize( 2048, 1080 );
    //settings->setWindowSize( 1080, 1920 );

    //settings->disableFrameRate();
    //settings->setFrameRate( 30 );
    settings->setResizable( true );
    settings->setFullScreen( false );

}


void DomainWarpingApp::resize()
{
    //hideCursor ();
}


void DomainWarpingApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'f' ) {
        // Toggle full screen.
        setFullScreen( !isFullScreen() );
    }
    else if( event.getCode() == app::KeyEvent::KEY_ESCAPE ) {
        if( isFullScreen() )
        setFullScreen( false );
        else
        quit();
    }
    else if( event.getChar() == 's' )
    {
        mStopZoom = true;
    }
    else if( event.getChar() == 'a' )
    {
        mStopZoom = false;
    }
    else if( event.getChar() == 'S' )
    {
        mStartShade = !mStartShade;
    }
    else if( event.getChar() == '1' && mNoiseScaleLimit > -1900.0f)
    {
        mNoiseScaleLimit -= 100.0f;
    }
    else if( event.getChar() == '2' && mNoiseScaleLimit < 1900.0f)
    {
        mNoiseScaleLimit += 100.0f;
    }
    else if( event.getChar() == '3' && mDeformationScaleLimit > -3.0f)
    {
        mDeformationScaleLimit -= 0.1f;
    }
    else if( event.getChar() == '4' && mDeformationScaleLimit < 3.0f)
    {
        mDeformationScaleLimit += 0.1f;
    }
    
    else if( event.getChar() == '5' && mXdeformationLimit > -2.0f)
    {
        mXdeformationLimit -= 0.1f;
        //console() <<  mXdeformationLimit;
    }
    else if( event.getChar() == '6' && mXdeformationLimit < 2.0f)
    {
        mXdeformationLimit += 0.1f;
        //console() <<  mXdeformationLimit;
    }
    
    else if( event.getChar() == '7' && mYdeformationLimit > -2.0f)
    {
        mYdeformationLimit -= 0.1f;
        //console() <<  mDeformationScaleLimit;
    }
    else if( event.getChar() == '8' && mYdeformationLimit < 2.0f)
    {
        mYdeformationLimit += 0.1f;
        //console() <<  mDeformationScaleLimit;
    }
    else if( event.getChar() == 'R')
    {
        mXdeformationLimit = 2.0f;
        mYdeformationLimit = 0.0f;
        mDeformationScaleLimit = 0.0f;
        //console() <<  mDeformationScaleLimit;
    }
}


void DomainWarpingApp::setup()
{
    //hideCursor ();
    
    mCam.lookAt( vec3( 0, 0, 1 ), vec3( 0 ) );
    mRect.texCoords(vec2(0.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f));
    
    // getting max Anisotropic maximum sampling available on the graphics card above 1
    mMaxAnisoFilterAmount = gl::Texture::getMaxAnisotropyMax() - 1.0f;
    
    try {
        mSNoiseCurlGlsl = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
        console() <<  "Loaded shader";
    }
    catch( const std::exception& e ) {
        console() <<  "Shader Error: " << e.what();
    }

    try {
        mFxaaShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "fxaa.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Fxaa shader: " << exc.what() );
    }
    
    gl::enableDepthWrite();
    gl::enableDepthRead();

    perlinParams = params::InterfaceGl::create( getWindow(), "Perlin settings", toPixels( ivec2( 230, 220 ) ) );
    perlinParams->addParam( "Frame rate", &mFrameRate );
    perlinParams->addSeparator();
    perlinParams->addParam( "NoiseScale", &mNoiseScale, "min=-1000.0 max=1000.0 step=0.01 keyIncr=q keyDecr=w" );
    perlinParams->addParam( "mDeformationScale", &mDeformationScale, "min=-3.0 max=3.0 step=0.0001 keyIncr=q keyDecr=w" );
   
    perlinParams->addSeparator();
    perlinParams->addParam( "Xdeformation", &Xdeformation, "min=-5.0 max=5.0 step=0.001 keyIncr=q keyDecr=w" );
    perlinParams->addParam( "Ydeformation", &Ydeformation, "min=-5.0 max=5.0 step=0.001 keyIncr=q keyDecr=w" );
    
    perlinParams->addSeparator();
    perlinParams->addParam( "mZoom", &mZoom, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    
    // FBO FOR PING PONG ONLY POSITION
    // surface setup with alpha channel
    perlinTexure1 = Surface32f(SIDE1, SIDE2, true);
    
    // Init surface
    Surface32f::Iter pixelIter = perlinTexure1.getIter();
    while( pixelIter.line() )
    {
        while( pixelIter.pixel() )
        {
            perlinTexure1.setPixel(pixelIter.getPos(), ColorAf( Rand::randFloat(0.0f, 1.0f), Rand::randFloat(0.0f, 1.0f), Rand::randFloat(0.0f, 1.0f) ));
        }
    }
    
    // Ping Pong vars
    mCurrentFBO = 0;
    mOtherFBO = 1;
    
    gl::Texture2d::Format velocity = gl::Texture2d::Format()
    .internalFormat( GL_RGBA32F_ARB )
    .magFilter( GL_LINEAR )
    .minFilter( GL_NEAREST )
    .wrap( GL_REPEAT );
    
    gl::Texture2d::Format tFormat;

    tFormat.setInternalFormat( GL_RGBA );
    tFormat.setWrap( GL_REPEAT, GL_REPEAT );
    tFormat.setMinFilter( GL_LINEAR );
    tFormat.setMagFilter( GL_NEAREST );
    tFormat.setMaxAnisotropy(mMaxAnisoFilterAmount);
    tFormat.mipmap();
    
    mPosition = gl::Texture::create( perlinTexure1, tFormat );
    
    tFormat.enableMipmapping( true );
    
    
    // Attachement 0 - position
    try {
        gl::Fbo::Format fboFormat;
        mTextureFboBuffer[ 0 ] = gl::Texture2d::create( perlinTexure1, tFormat );
        mTextureFboBuffer[ 1 ] = gl::Texture2d::create( perlinTexure1, tFormat );
        
        fboFormat.attachment( GL_COLOR_ATTACHMENT0, gl::Texture2d::create( perlinTexure1, tFormat ) );
        fboFormat.attachment( GL_COLOR_ATTACHMENT1, gl::Texture2d::create( SIDE1, SIDE2, velocity ) );
        mFboBufferTest[ 0 ] = gl::Fbo::create( SIDE1, SIDE2, fboFormat );
    }
    catch( const std::exception& e ) {
        console() << "mFboGBuffer failed: " << e.what() << std::endl;
    }
    
    // Attachement 1 - velocity
    try {
        gl::Fbo::Format fboFormat;
        fboFormat.setSamples(4);
        mTextureFboBuffer2[ 0 ] = gl::Texture2d::create( perlinTexure1, tFormat );
        mTextureFboBuffer2[ 1 ] = gl::Texture2d::create( perlinTexure1, tFormat  );
        
        fboFormat.attachment( GL_COLOR_ATTACHMENT0, gl::Texture2d::create( perlinTexure1, tFormat ));
        fboFormat.attachment( GL_COLOR_ATTACHMENT1, gl::Texture2d::create( SIDE1, SIDE2, velocity ));
        
        mFboBufferTest[ 1 ] = gl::Fbo::create( SIDE1, SIDE2, fboFormat );
    }
    catch( const std::exception& e ) {
        console() << "mFboGBuffer failed: " << e.what() << std::endl;
    }
    
    // Set up the buffer for rendering
    {
        ivec2 sz = ivec2( SIDE1, SIDE2 );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 1; ++i ) {
            mTexturemRender[ i ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                         .internalFormat( GL_RGBA32F_ARB )
                                                         .magFilter( GL_LINEAR )
                                                         .minFilter( GL_LINEAR )
                                                         .wrap( GL_CLAMP_TO_EDGE )
                                                         .dataType( GL_FLOAT )
                                                         .mipmap());
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTexturemRender[ i ] );
        }
        
        mRenderFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        gl::clear();
    }
    
    // BATCHS
    gl::VboMeshRef  rect = gl::VboMesh::create( geom::Rect() );
    
    // Fxaa Batch
    mBatchFxaaRect = gl::Batch::create( rect, mFxaaShader );
    mBatchFxaaRect->getGlslProg()->uniform(	"uSampler",	0 );
    
    // To screen Batch
    gl::GlslProgRef stockTexture    = gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchToScreenRect              = gl::Batch::create( rect, stockTexture );
}


void DomainWarpingApp::mouseDown( MouseEvent event )
{

}


void DomainWarpingApp::update()
{
    //hideCursor();
    
    // Other Parameters
    mFrameRate = getAverageFps();
    
    // Track current time so we can calculate elapsed time.
    mCurrentSeconds1 = mCounter_1;
    mCurrentSeconds2 = sin ( mCounter_1*0.01 );

    if( mZoomValue <= -4.0f)
    {
        mZoomIn = true;
        mCycleCounter += 1;
    }
    
    if( mZoomValue >= 4.0f)
    {
        mZoomIn = false;
        mCycleCounter += 1;
    }
    /*if( mZoomValue >= -0.7f)
    {
        mZoomIn = false;
    }*/
    
    if( mZoomIn == true && mZoomValue < 4.0f  && mStopZoom == false )
    {
        if ( mCycleCounter < 3 ) mZoomValue += 0.003478f;
        else mZoomValue += 0.003663f;
    }
    
    if( mZoomIn == false && mZoomValue > -4.0f && mStopZoom == false )
    {
        if ( mCycleCounter < 3 ) mZoomValue -= 0.003478f;
        mZoomValue -= 0.003663f;

    }
    //console() << "mZoomValue : " << mCycleCounter << std::endl;

    if( mStartShade == true && mShade > 0.0f )
    {
        mShade -= 0.0001f;
    }
    
    if( mNoiseScale > mNoiseScaleLimit && mCycleCounter >= 3 )
    {
        mNoiseScale -= 0.03f;
    }
    else if( mNoiseScale < mNoiseScaleLimit && mCycleCounter >= 3 )
    {
        mNoiseScale += 0.03f;
    }
    
    if( mDeformationScale > mDeformationScaleLimit )
    {
        mDeformationScale -= 0.00001f;
    }
    else if( mDeformationScale < mDeformationScaleLimit-0.0001f )
    {
        mDeformationScale += 0.00001f;
    }
    
    if( mZoomValue < 2.5f && mZoomValue > 0.0f )
    {
        Xdeformation += 0.001f;
    }
    else if( mZoomValue > -2.5f && mZoomValue < 0.0f )
    {
        Xdeformation -= 0.001f;
    }
    
    /*if( mZoomValue > -2.5f && mZoomIn == true )
    {
        Xdeformation += 0.003f;
    }
    else if( mZoomValue > -2.5f && Xdeformation > 0.0f && mZoomIn == false )
    {
        Xdeformation -= 0.005f;
    }*/
    
    if( Ydeformation > mYdeformationLimit  )
    {
        Ydeformation -= 0.001f;
    }
    else if( Ydeformation < mYdeformationLimit-0.0001f )
    {
        Ydeformation += 0.00001f;
    }
    
    if( mCycleCounter >= 3 )
    {
        mSaveScreen = true;
    }

    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::ScopedFramebuffer scpFbo( mFboBufferTest[mCurrentFBO] );
    gl::ScopedViewport    scpViewport(ivec2( 0 ), mFboBufferTest[mCurrentFBO]->getSize() );
    
    gl::clear( ColorA( 0, 0, 0, 0 ) );
    
    gl::ScopedMatrices scpMatrices;
    gl::setMatricesWindow( mFboBufferTest[mCurrentFBO]->getSize() );
    
    gl::ScopedTextureBind scpTex0( mFboBufferTest[mOtherFBO]->getTexture2d( GL_COLOR_ATTACHMENT0 ), 0 );
    gl::ScopedTextureBind scpTex1( mFboBufferTest[mOtherFBO]->getTexture2d( GL_COLOR_ATTACHMENT1 ), 1 );
    
    gl::ScopedGlslProg glslScope( mSNoiseCurlGlsl );
    mSNoiseCurlGlsl->uniform( "tex", 0 );
    mSNoiseCurlGlsl->uniform( "tex2", 1 );
    //mSNoiseCurlGlsl->uniform( "noisescale",	mCurrentSeconds2 * mNoiseScale );
    mSNoiseCurlGlsl->uniform( "noisescale",	mNoiseScale );

    mSNoiseCurlGlsl->uniform( "xdeformation", Xdeformation );
    mSNoiseCurlGlsl->uniform( "ydeformation", Ydeformation );
    mSNoiseCurlGlsl->uniform( "zoom", mZoomValue );
    //mSNoiseCurlGlsl->uniform( "time", mCurrentSeconds1 * mDeformationScale );
    mSNoiseCurlGlsl->uniform( "time", mDeformationScale );
    if( mZoomIn == false ) mSNoiseCurlGlsl->uniform( "iTime", (float)-mCounter_1*0.1f );
    else if( mZoomIn == true ) mSNoiseCurlGlsl->uniform( "iTime", (float)-mCounter_1*0.1f );


    mSNoiseCurlGlsl->uniform( "iResolution", vec3(mFboBufferTest[mCurrentFBO]->getSize(), 0.0f) );
    mSNoiseCurlGlsl->uniform( "shade", mShade );
    
    
    gl::drawSolidRect( mFboBufferTest[mCurrentFBO]->getBounds() );
    
    mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
    mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
    
}


void DomainWarpingApp::draw()
{
    // Fxaa
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mRenderFBO->getSize() );
        gl::translate( mRenderFBO->getSize() / 2 );
        gl::scale( mRenderFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        gl::clear( Color( 0, 0, 0 ) );
        // To keep bandwidth in check, we aren't using any hardware
        // anti-aliasing (MSAA). Instead, we use FXAA as a post-process
        // to clean up our image.
        const gl::ScopedTextureBind scopedTextureBind0( mFboBufferTest[mCurrentFBO]->getTexture2d( GL_COLOR_ATTACHMENT1 ), 0 ); // final render texture
        mBatchFxaaRect->getGlslProg()->uniform( "uPixel", 1.0f / vec2( mRenderFBO->getSize() ) );
        mBatchFxaaRect->draw();
    }
    
    gl::viewport( toPixels( getWindowSize() ) );
    gl::clear( Color::gray( 0.35f ) );
    gl::setMatricesWindow( getWindowSize() );
    
    // draw the two textures we've created side-by-side
    auto tex0 = mFboBufferTest[mCurrentFBO]->getTexture2d( GL_COLOR_ATTACHMENT0 );
    auto tex1 = mFboBufferTest[mCurrentFBO]->getTexture2d( GL_COLOR_ATTACHMENT1 );
    auto tex2 = mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT0 );

    gl::draw( tex2, getWindowBounds());

    perlinParams->draw();
    
    // Save render fbo
    if ( mSaveScreen == true ){
        saveImage();
    }
}


// Save renderFbo
void DomainWarpingApp::saveImage()
{
    //console() << "mCurrentFrame : " << mCurrentFrame << std::endl;

    if (  ( mSaveScreen == true && mCounter_1 < 145.0f ))
    {
        // console() << "mCurrentFrame: " << mCurrentFrame << std::endl;
        
        //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mFrameCounter ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT0 )->createSource(), ImageTarget::Options().quality(1.0f) );
        mFrameCounter++;
    }

}


CINDER_APP( DomainWarpingApp, RendererGl( RendererGl::Options().msaa( 16 ) ), &DomainWarpingApp::prepareSettings )
