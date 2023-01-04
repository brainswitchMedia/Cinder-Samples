#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/CameraUi.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"

#include "ReactiveCam.h"
#include "XMLcom.h"

#undef PHI	// take the reciprocal of phi, to obtain an icosahedron that fits a unit cube
#define PHI (1.0f / ((1.0f + math<float>::sqrt(5.0f)) / 2.0f))
#define HALFPHI 3.14159265359f / 2.0f
#define XML TRUE

const float PI = 3.14159265359;

using namespace ci;
using namespace ci::app;
using namespace std;


struct LightData {
    bool						toggleViewpoint;
    float						distanceRadius;
    float						fov;
    CameraPersp					camera;
    vec3						viewpoint;
    vec3						target;
};


class Blockletter01App : public App {
public:
    
    // METHODS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Blockletter01App();
    void            prepareSettings( App::Settings* settings );
	void            setup() override;
	void            update() override;
	void            draw() override;
    void            resize() override;
    void            mouseDown( MouseEvent event ) override;
    void            mouseDrag( MouseEvent event ) override;
    void            keyDown( KeyEvent event ) override;
    
    void            createPrimitivesPositionsFromCube();
    gl::VboMeshRef  createPrimitivePyramidFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4, bool cavity );
    vec3            computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 );
    vec3            computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 );
    vec3            rotateAxis( bool beat, vec3 &rotationVectorFrom, vec3 &rotationVectorTo, float easeSpeed );
    vec3            slerp( vec3 &start, vec3 &end, float percent );
    mat4            rotationAlign( vec3 d, vec3 z );
    float           linearize( float a0, float a1, float b0, float b1, float in );


    void loadShaders();
    void loadTextures();
    void saveImage();
    void createGeometry();
    void createParams();
    void setXMLAnimationPhases();
    void transition_0( bool start );
    void transition_1( bool start );
    void transition_2( bool start );
    
    
    // VARS /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Tetraedre buffers
    std::vector<uint16_t>       mIndices;
    std::vector<ci::vec3>       mPosCoords;
    std::vector<uint16_t>       mCheckedIndices;
    std::vector<ci::vec3>       mUniquePosCoords;
    
    // Tetraedre vars
    vec3                        mUnitCube[8];
    int                         mSubdivisions = 2;
    
    // Pyramids buffer
    gl::BatchRef                mBatchSphere, mBatchPyramid;
    
    // GLSL & Rendering
    gl::BatchRef                mBatchGodRays, mBatchTrailRect, mBatchBloomRect, mBatchToScreenRect, mBatchHdrRect, mBatchFxaaRect;
    gl::GlslProgRef             mWireframeShader, mColorShader, mTrailShader, mGodRaysShader, mBloomShader, mHdrShader, mFxaaShader;
    
    // FBO
    gl::FboRef                  mRenderFBO, mTrailFBO, mGodRaysFBO, mBloomFBO, mHdrFBO;
    ci::gl::Texture2dRef        mTexturemRender[ 6 ];
    ci::gl::Texture2dRef		mTextureFboTrail[ 4 ];
    ci::gl::Texture2dRef        mTextureGodRays[ 1 ];
    ci::gl::Texture2dRef        mTextureHdr[ 1 ];
    ci::gl::Texture2dRef        mTextureBloom[ 2 ];
    ci::gl::TextureRef          mTextureStripes, mTextureNormal, mTexRoughness;
    
    // FBO Size
    //vec2                        mFboSize = vec2 (3360, 2100);
    //vec2                        mFboSize = vec2 ( 1920, 1080 );
    //vec2                        mFboSize = vec2 (1080 , 1920);
    vec2                        mFboSize = vec2 (1080, 1080);
    //vec2                        mFboSize = vec2 (3840, 3840);
    //vec2                        mFboSize = vec2 (3840, 2160);
    //vec2                        mFboSize = vec2 ( 540, 540 );
    //vec2                        mFboSize = vec2 ( 270, 270 );
    
    // Drawing Objects
    float                       mBrightness;
    float                       mPyramidsIlluminationSize;
    float                       mLightPower;
    float                       mDistancePower1;
    float                       mDistancePower2;
    float                       mCircleSize;
    float                       mCircleSize2;
    
    // Distance shader
    LightData                   mLightCamera;
    float                       mDistanceConverstionTetraedre1;
    float                       mDistanceConverstionTetraedre2;

    
    // Bloom
    float                       aBloomValues[ 5 ];
    
    // GodRays
    float                       mExposure;
    float                       mDecay;
    float                       mDensity;
    float                       mWeight;
    
    // TRAILS
    size_t                      ping;
    size_t                      pong;
    float                       mTrailAttenuation;
    
    // HDR
    float                       mHdrExposure;
    float                       mHdrGamma;
    float                       mMixBloomGodrays;
    float                       mChromaticAberration;
    
    // Camera
    vec3                        mCameraTarget = vec3( 0.0, 0.0, 0.0 );
    ReactiveCam                 mReactiveCam;
    bool                        mIsBeat, mIsBeat2;
    float                       mbeatCounter;
    float                       mRotationAcc;
    
    // Scene Rotation
    mat4                        mSceneRotation;
    float                       mSceneRotationSpeed;
    vec3                        rotationVector1;
    vec3                        rotationVector2;
    int                         mChangeRotation;
    float                       mCounterCycle;
    
    // Colors
    vec3                        mColor1;
    vec3                        mColor2;

    // SOUND XML PARSER //////////////////////////////////////////
    XMLcom                      mXMLcom_Bass, mXMLcom_Medium;
    bool                        mPlayXML;
    float                       mSoundEnergyHeightFactor, mSoundEnergy2;
    float                       mIncrease = 1.0f;
    float                       mbeatCounter_1 = 1.0f;
    float                       mbeatCounter_2 = 1.0f;
    float                       mbeatCounter_3 = 1.0f;
    float                       mEnergyCoef_1 = 0.2f;
    float                       mHdrGammaCounter = 0.0f;
    
    // Animation
    float                       mCounter_1;
    bool                        mStart_Transition0, mStart_Transition1, mStart_Transition2;
    
    // Params
    params::InterfaceGlRef      mParams;
    std::string                 mKeyPressed;
    float                       mFrameRate;
    
    // SAVE IMAGE ///////////////////////////////////////////////
    int                         mCurrentFrame = 0;
    int                         mFrameCounter = 0;
    bool                        mSaveScreen = true;
    bool                        mLowfpsXML = false;
};


Blockletter01App::Blockletter01App()
{
    // Centered unit cube: vertex coordinates with edde = 2.0
    //      3-----7         y
    //      |     |         |__x
    //   2-----6  |       z/
    //      4.....8
    //
    //   1-----5
    
    mUnitCube[0] = vec3( -1.0, -1.0, 1.0 ); //1 - Vtet3
    mUnitCube[1] = vec3( -1.0, 1.0, 1.0 ); //2
    mUnitCube[2] = vec3( -1.0, 1.0, -1.0 ); //3 - Vtet2
    mUnitCube[3] = vec3( -1.0, -1.0, -1.0 ); //4
    mUnitCube[4] = vec3( 1.0, -1.0, 1.0 ); // 5
    mUnitCube[5] = vec3( 1.0, 1.0, 1.0 ); // 6 - Vtet0
    mUnitCube[6] = vec3( 1.0, 1.0, -1.0 ); // 7
    mUnitCube[7] = vec3( 1.0, -1.0, -1.0 ); // 8 - Vtet1
}


void prepareSettings( App::Settings* settings )
{
    settings->setWindowSize( 1080, 1080 );
    settings->setHighDensityDisplayEnabled();
    settings->setMultiTouchEnabled( false );
    settings->disableFrameRate();
}


void Blockletter01App::setup()
{
    mFrameRate = 0;
    
    // Setup animation variables
    mCounter_1  = 0.0f;
    mCounterCycle = 0.0f;
    
    // Drawing Objects
    mBrightness = 0.059f;
    mPyramidsIlluminationSize = 1.0f;
    mLightPower = 50.0f;
    mDistancePower1 = -0.35f;
    mDistancePower2 = -0.23f;
    mCircleSize = 2.15;
    mCircleSize2 = 2.0;
    
    // Ligthdata
    mLightCamera.distanceRadius     = 20.0f;
    mLightCamera.viewpoint          = vec3( mLightCamera.distanceRadius );
    mLightCamera.fov                = 60.0f;
    mLightCamera.target             = vec3( 0 );
    mLightCamera.toggleViewpoint	= false;
    mDistanceConverstionTetraedre1  = 11.4f;
    mDistanceConverstionTetraedre2  = 10.2f;
    
    // Bloom
    aBloomValues[0] = 0.227027f;
    aBloomValues[1] = 0.1945946f;
    aBloomValues[2] = 0.1216216f;
    aBloomValues[3] = 0.054054f;
    aBloomValues[4] = 0.016216f;
    
    // God Rays
    mExposure   = 0.06f;
    mDecay      = 0.98f;
    mDensity    = 0.01f;
    mWeight     = 0.01f;
    
    // Trails
    ping = 0;
    pong = 1;
    mTrailAttenuation = 0.078f;
    
    // HDR
    mMixBloomGodrays        = 0.0f;
    mHdrExposure            = 1.0f;
    mHdrGamma               = 0.5f;
    mChromaticAberration    = 0.0012f;
    
    // Scene rotation
    mRotationAcc = 0.07f;
    mChangeRotation = 0;
    mSceneRotationSpeed = 0.01f;
    rotationVector1 = normalize ( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
    vec3 rotationVectorTmp = normalize ( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
    rotationVector2 = cross( rotationVector1, rotationVectorTmp );
    
    // ReactiveCam
    mIsBeat = false;
    mIsBeat2 = false;
    mReactiveCam.set( 6.5f, mCameraTarget, 0.01f, 10.0f );
    mLightCamera.camera.setPerspective( mLightCamera.fov, getWindowAspectRatio(), 0.01, 10.0 ); // 1000
    mReactiveCam.setKeyEvent( mKeyPressed );
    mIsBeat = false;

    // Colors
    mColor1 = vec3( 1.00, 0.15, 0.28 );
    mColor2 = vec3( 0.3, 0.385, 0.315 );
    
    // Load and compile the shaders
    loadShaders();
    
    // Load textures
    loadTextures();
    
    // Setup objects position
    createPrimitivesPositionsFromCube();
    
    // Create the meshes.
    createGeometry();
    
    // connect the keydown signal
    mKeyPressed = "nul";
    
    getWindow()->getSignalKeyDown().connect( [this]( KeyEvent event )
                                            {
                                                switch ( event.getCode() )
                                                {
                                                    case KeyEvent::KEY_s :
                                                        console() << "mCurrentFrame: " << mCurrentFrame << std::endl;
                                                        mSaveScreen = !mSaveScreen;
                                                        break;
                                                    case KeyEvent::KEY_f :
                                                        setFullScreen( !isFullScreen() );
                                                        resize();
                                                        break;
                                                    case KeyEvent::KEY_1 :
                                                        mStart_Transition0 = true; mStart_Transition1 = false;
                                                        mStart_Transition2 = false;
                                                        break;
                                                    case KeyEvent::KEY_2 :
                                                        mStart_Transition0 = false; mStart_Transition1 = false;
                                                        mStart_Transition2 = true;
                                                        break;
                                                    case KeyEvent::KEY_3 :
                                                        break;
                                                        // Start read from xml
                                                    case KeyEvent::KEY_p :
                                                        if ( XML == 1 ) {
                                                            mPlayXML = true;
                                                            mSaveScreen = true;
                                                            console() << "mCurrentFrame: " << mCurrentFrame << std::endl;
                                                            console() << "start elapsed time " << getElapsedSeconds() << std::endl;
                                                        }
                                                        break;
                                                        
                                                        // Stop read from xml
                                                    case KeyEvent::KEY_l :
                                                        if ( XML == 1 ) {
                                                            console() << "stop elapsed time " << getElapsedSeconds() << std::endl;
                                                            mSaveScreen = false;
                                                            mPlayXML = false;
                                                        }
                                                        break;
                                                }
                                            });
    
    // Resize window
    resize();
    
    // Create a parameter window, so we can toggle stuff.
    createParams();
    
    // COM
    mPlayXML = false;

    console() << "stop elapsed time " << getElapsedSeconds() << std::endl;
    mXMLcom_Bass.start( "low.xml" );
    mXMLcom_Medium.start( "mid.xml" );

    
    // Transitions
    mStart_Transition0 = false;
    mStart_Transition1 = false;
    mStart_Transition2 = false;

}


void Blockletter01App::update()
{
    // Other Parameters
    mFrameRate = getAverageFps();
    
    // COM
    if ( mPlayXML == TRUE && mCurrentFrame > 968 )
    {
        mXMLcom_Bass.update();
        mXMLcom_Medium.update();
        setXMLAnimationPhases();
    }
    
    // Transitions
    transition_0( mStart_Transition0 );
    transition_1( mStart_Transition1 );
    transition_2( mStart_Transition2 );
    
    // Scene rotation
    vec3 rotationAxis = rotateAxis( mIsBeat2, rotationVector1, rotationVector2, 0.00f );
    mSceneRotation *= rotate( -mSceneRotationSpeed, rotationAxis );
    
    // Update Camera
    mReactiveCam.update( mIsBeat, normalize( vec3( 0.5f, -1.5f, 0.5f ) ), true, mRotationAcc );
    mLightCamera.viewpoint = mLightCamera.distanceRadius * normalize( mReactiveCam.getPosition() ) * -1.0f;
    mLightCamera.camera.lookAt( mLightCamera.viewpoint, mLightCamera.target );
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
    mCounterCycle += 0.01667f;
    
    // 30 fps
    //mCounter_1 += 0.03333f;
    
    // Beat
    mIsBeat = false;
    mIsBeat2 = false;
    
    //mDistancePower1 = -0.35 + 0.2 * sin( 1.5 * mCounter_1 );
}


void Blockletter01App::draw()
{
    gl::enableVerticalSync( true );
    
    // Render scene in 2 textures ( GL_COLOR_ATTACHMENT1 for the bloom pass )
    {
        // Enable the depth buffer.
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::disableBlending();
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        
        // Set Draw buffers
        {
            const static GLenum buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
            };
            gl::drawBuffers( 3, buffers );
        }
        
        gl::clear();
        
        // Set Matrices
        gl::setMatrices( mReactiveCam.getCameraMatrix() );
        gl::ScopedModelMatrix modelScope;
        
        // => DRAW OBJECTS HERE
        // Render Sphere
        {
            mColorShader->uniform( "uLineStyle", 1 );
            mColorShader->uniform( "uColor", mColor2 );
            mColorShader->uniform( "uSize", 0.33f );
            mColorShader->uniform( "uPower1", mDistancePower2 );
            mColorShader->uniform( "uWorldtoLightMatrix", mLightCamera.camera.getViewMatrix() );
            mColorShader->uniform( "uLightModelView", mReactiveCam.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
            mColorShader->uniform( "uDistanceConverstion", mDistanceConverstionTetraedre2 );
            mColorShader->uniform( "uFront", true );
            mColorShader->uniform( "uBrightness", 0.5f );
            mColorShader->uniform( "uCircleSize", mCircleSize2 );

            // Render Frontside only
            gl::ScopedFaceCulling cullScope( true, GL_BACK );
            gl::cullFace( GL_BACK );
            //mBatchPyramid->draw();

            mBatchSphere->draw();
        }
        
        {
            gl::ScopedTextureBind scopedTextureBind0( mTextureNormal, 0 );
            gl::ScopedTextureBind scopedTextureBind2( mTexRoughness, 1 );
            mWireframeShader->uniform( "uLineStyle", 1 );
            mWireframeShader->uniform( "uColor", mColor1 );
            mWireframeShader->uniform( "uSize", mPyramidsIlluminationSize );
            mWireframeShader->uniform( "uPower1", mDistancePower1 );
            mWireframeShader->uniform( "uWorldtoLightMatrix", mLightCamera.camera.getViewMatrix() );
            // Ligth position in view space for normal mapping  illumination
            mWireframeShader->uniform( "uLightPosition", vec3( mReactiveCam.getViewMatrix() * vec4( 0.5f * mReactiveCam.getPosition(), 1.0 )) );
            mWireframeShader->uniform( "uCircleSize", mCircleSize );

            
            // Render the back side first.
            gl::multModelMatrix( mSceneRotation );
            
            gl::ScopedBlendAlpha blendScope;

             gl::ScopedFaceCulling cullScope( true,   GL_FRONT);
             {
                 mWireframeShader->uniform( "uLightModelView", mReactiveCam.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
                 // For the back we need less attenuation
                 mWireframeShader->uniform( "uDistanceConverstion", mDistanceConverstionTetraedre1 );
                 mWireframeShader->uniform( "uFront", false );
                 mWireframeShader->uniform( "uBrightness", 0.5f );
                 mColorShader->uniform( "uTime", 0.05f * mCounter_1 );

                 mBatchPyramid->draw();
             }
            
            gl::cullFace( GL_BACK );
            {
                mWireframeShader->uniform( "uLightModelView", mReactiveCam.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
                mWireframeShader->uniform( "uDistanceConverstion", 0.0f );
                mWireframeShader->uniform( "uFront", true );
                mWireframeShader->uniform( "uBrightness", 0.5f );
                mBatchPyramid->draw();
            }
        }
    }
        
        // GodRays
    /*{
        vec2 mLightShaderPos = mReactiveCam.getWorldToScreen( vec3( 0.0f, 0.0f, 0.0f ), mFboSize.x, mFboSize.y );
            
        mLightShaderPos.x /= mFboSize.x;
        mLightShaderPos.y /= mFboSize.y;
        mLightShaderPos.y = 1.0f - mLightShaderPos.y;
        
        const gl::ScopedFramebuffer scopedFrameBuffer( mGodRaysFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mGodRaysFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mGodRaysFBO->getSize() );
        gl::translate( mGodRaysFBO->getSize() / 2 );
        gl::scale( mGodRaysFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        gl::clear( Color( 0, 0, 0 ) );
        
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 1 ], 0 );
        mBatchGodRays->getGlslProg()->uniform( "uExposure", mExposure );
        mBatchGodRays->getGlslProg()->uniform( "uDecay", mDecay );
        mBatchGodRays->getGlslProg()->uniform( "uDensity", mDensity );
        mBatchGodRays->getGlslProg()->uniform( "uWeight", mWeight );
        mBatchGodRays->getGlslProg()->uniform( "uLightPositionOnScreen", mLightShaderPos );
        mBatchGodRays->draw();
    }*/
        
    // Bloom
    // Ping pong bloom vars
    bool horizontal = true;
    int horizontalPass = 1;
    bool first_iteration = true;
    int numberOfPasses = 6;
    {
         const gl::ScopedFramebuffer scopedFrameBuffer( mBloomFBO );
         const gl::ScopedViewport scopedViewport( ivec2( 0 ), mBloomFBO->getSize() );
         const gl::ScopedMatrices scopedMatrices;
         gl::setMatricesWindow( mBloomFBO->getSize() );
         gl::disableDepthRead();
         gl::disableDepthWrite();
         gl::translate( (float)mBloomFBO->getWidth()/2.0f, (float)mBloomFBO->getHeight()/2.0f );
         gl::scale( mBloomFBO->getSize() );
         
         // Run horizontal blur pass and vertical blur pass
         for ( int i = 0; i < numberOfPasses; i++ )
         {
             gl::drawBuffer( GL_COLOR_ATTACHMENT0  + (GLenum)horizontal );
             gl::clear( Color( 0, 0, 0 ) );
             int height = mBloomFBO->getHeight();
             int width = mBloomFBO->getWidth();
         
             if ( first_iteration )
             {
                 mTexturemRender[ 2 ]->bind(0);
                 //mTexturemRender[ 1 ]->bind(0);
             }
             else mTextureBloom[(int)!horizontal]->bind(0);
         
             mBatchBloomRect->getGlslProg()->uniform( "uWeight", aBloomValues, 5 );;
             mBatchBloomRect->getGlslProg()->uniform( "uHorizontal", horizontalPass );
             //mBatchBloomRect->getGlslProg()->uniform( "uWidth", height );
             //mBatchBloomRect->getGlslProg()->uniform( "uHeight", width );
             mBatchBloomRect->draw();
         
             if ( first_iteration ) mTexturemRender[2]->unbind(0);
             else mTextureBloom[(int)!horizontal]->unbind(0);
         
             if ( horizontalPass == 0 ) horizontalPass = 1;
             else if ( horizontalPass == 1 ) horizontalPass = 0;
         
             horizontal = !horizontal;
             if ( first_iteration )
             first_iteration = false;
         }
    }

    
    // Trails
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mTrailFBO );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0  + (GLenum)ping );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mTrailFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mTrailFBO->getSize() );
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( mTrailFBO->getSize() / 2 );
        gl::scale( mTrailFBO->getSize() );
        
        // Dim the light accumulation buffer to produce trails.
        //  Lower alpha makes longer trails and paint light sources.
        {
            const gl::ScopedTextureBind scopedTextureBind0( mTextureFboTrail[ pong ], 0 );
            //const gl::ScopedTextureBind scopedTextureBind1( mTextureBloom[(int)!horizontal], 1 );
            const gl::ScopedTextureBind scopedTextureBind1( mTexturemRender[ 1 ], 1 );
            
            mBatchTrailRect->getGlslProg()->uniform( "attenuation", mTrailAttenuation );
            mBatchTrailRect->draw();
        }
    }
    
    // Render our final image to the screen with hdr
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mRenderFBO->getSize() );
        gl::translate( mRenderFBO->getSize() / 2 );
        gl::scale( mRenderFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT3 );
        gl::clear( Color( 0, 0, 0 ) );
        // Blomm Not used
        const gl::ScopedTextureBind scopedTextureBind1( mTextureFboTrail[ ping ], 0 ); // trail texture
        const gl::ScopedTextureBind scopedTextureBind0( mTextureBloom[(int)!horizontal], 1 ); // blomm
        const gl::ScopedTextureBind scopedTextureBind2( mTexturemRender[ 0 ], 2 ); // Scene
        mBatchHdrRect->getGlslProg()->uniform( "uIsBloom", true );
        mBatchHdrRect->getGlslProg()->uniform( "uIsGodrays", false );
        mBatchHdrRect->getGlslProg()->uniform( "uIsgamma", true );
        mBatchHdrRect->getGlslProg()->uniform( "uMixBloomGodrays", mMixBloomGodrays );
        mBatchHdrRect->getGlslProg()->uniform( "uExposure", mHdrExposure );
        mBatchHdrRect->getGlslProg()->uniform( "uGamma", mHdrGamma );
        mBatchHdrRect->getGlslProg()->uniform( "uChromaticAberration", mChromaticAberration );
        mBatchHdrRect->draw();
    }
    
    // Fxaa
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mRenderFBO->getSize() );
        gl::translate( mRenderFBO->getSize() / 2 );
        gl::scale( mRenderFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT4 );
        gl::clear( Color( 0, 0, 0 ) );
        // To keep bandwidth in check, we aren't using any hardware
        // anti-aliasing (MSAA). Instead, we use FXAA as a post-process
        // to clean up our image.
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 3 ], 0 ); // final render texture
        mBatchFxaaRect->getGlslProg()->uniform( "uPixel", 1.0f / vec2( mRenderFBO->getSize() ) );
        mBatchFxaaRect->draw();
    }
    
    // Render to the screen
    {
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), toPixels( getWindowSize() ) );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( toPixels( getWindowSize() ) );
        gl::translate( toPixels( getWindowSize() / 2 )  );
        gl::scale( toPixels( getWindowSize() ) );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureBloom[(int)!horizontal], 0 ); // bloom texture
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureFboTrail[ ping ], 0 ); // trail texture
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureGodRays[ 0 ], 0 ); // trail texture
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 4 ], 0 );
        mBatchToScreenRect->draw();
    }

    // Trails ping pong
    ping = pong;
    pong = ( ping + 1 ) % 2;
    
    // Disable the depth buffer.
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    // Render the parameter windows.
    if( mParams ) {
        mParams->draw();
    }
    
    // Save render fbo
    if ( mSaveScreen == true ){
        saveImage();
    }
    
    mCurrentFrame++;
}


void Blockletter01App::resize()
{
    mLightCamera.camera.setAspectRatio( mFboSize.x/mFboSize.y );
    mReactiveCam.resize( vec2( mFboSize.x, mFboSize.y ) );
    
    // Choose window size based on selected quality
    ivec2 winSize       = toPixels( getWindowSize() );
    //int32_t h         = winSize.y;
    //int32_t w         = winSize.x;
    int32_t h           = mFboSize.y;
    int32_t w           = mFboSize.x;
    
    cinder::app::console() <<  "RESIZE >> Y:" << h << " Y:" << w;
    
    gl::VboMeshRef  rect = gl::VboMesh::create( geom::Rect() );
    
    // Set up the buffer for rendering
    // 0 GL_COLOR_ATTACHMENT1	scene
    // 1 GL_COLOR_ATTACHMENT1	scene for bloom
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 6; ++i ) {
            mTexturemRender[ i ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                         .internalFormat( GL_RGBA16F_ARB )
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
    
    // Ping Pong Trails Trail
    {
        // must adjust size to half the screen
        ivec2 sz = ivec2( w, h );
        //ivec2 sz = ivec2( w/2, h/2 );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 2; ++i ) {
            mTextureFboTrail[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                          .internalFormat( GL_RGBA16F_ARB )
                                                          .magFilter( GL_LINEAR )
                                                          .minFilter( GL_LINEAR )
                                                          .wrap( GL_CLAMP_TO_EDGE )
                                                          .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureFboTrail[ i ] );
        }
        mTrailFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mTrailFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mTrailFBO->getSize() );
        gl::clear();
    }
    
    // Bloom FBO
    {
        // must adjust size to half the screen
        //ivec2 sz = ivec2( w, h );
        ivec2 sz = ivec2( w/2, h/2 );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 2; ++i ) {
            mTextureBloom[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                       .internalFormat( GL_RGBA32F_ARB )
                                                       .magFilter( GL_LINEAR )
                                                       .minFilter( GL_LINEAR )
                                                       .wrap( GL_CLAMP_TO_EDGE )
                                                       .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureBloom[ i ] );
        }
        mBloomFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mBloomFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mBloomFBO->getSize() );
        gl::clear();
    }
    
    // Ligth Squatering FBO
    {
        // Adjust size to half of the screen
        ivec2 sz = ivec2( w, h );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 1; ++i ) {
            mTextureGodRays[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                         .internalFormat( GL_RGBA16F_ARB )
                                                         .magFilter( GL_LINEAR )
                                                         .minFilter( GL_LINEAR )
                                                         .wrap( GL_CLAMP_TO_EDGE )
                                                         .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureGodRays[ i ] );
        }
        mGodRaysFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mGodRaysFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mGodRaysFBO->getSize() );
        gl::clear();
    }
    
    // HDR FBO
    {
        // Adjust size to half of the screen
        ivec2 sz = ivec2( w, h );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 1; ++i ) {
            mTextureHdr[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                     .internalFormat( GL_RGBA16F_ARB )
                                                     .magFilter( GL_LINEAR )
                                                     .minFilter( GL_LINEAR )
                                                     .wrap( GL_CLAMP_TO_EDGE )
                                                     .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureHdr[ i ] );
        }
        mHdrFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mHdrFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mHdrFBO->getSize() );
        gl::clear();
    }
}


void Blockletter01App::createPrimitivesPositionsFromCube()
{
    mPosCoords.clear();
    mIndices.clear();
    
    // Wikipedia
    // Tetrahedron: (1,1,1), (1,−1,−1), (−1,1,−1), (−1,−1,1)
    // Dual tetrahedron: (−1,−1,−1), (−1,1,1), (1,−1,1), (1,1,−1)
    
    // Vertex
    //
    //     V2-----*         y
    //      :     |         |__x
    //   *-----V0 |       z/
    //      *.....V1
    //
    //  V3-----*
    
    // If subdivide
    mPosCoords.push_back( mUnitCube[5] );
    mPosCoords.push_back( mUnitCube[7] );
    mPosCoords.push_back( mUnitCube[2] );
    mPosCoords.push_back( mUnitCube[0] );
    mIndices.push_back( 1 );
    mIndices.push_back( 2 );
    mIndices.push_back( 0 );
    mIndices.push_back( 2 );
    mIndices.push_back( 3 );
    mIndices.push_back( 0 );
    mIndices.push_back( 3 );
    mIndices.push_back( 1 );
    mIndices.push_back( 0 );
    mIndices.push_back( 1 );
    mIndices.push_back( 3 );
    mIndices.push_back( 2 );
    
    // Normals: calculate the face normal for each triangle
    size_t numTriangles = mIndices.size() / 3;
    console() << "numTriangles: " << numTriangles << std::endl;
    
    size_t mPosCoordszise = mPosCoords.size();
    
    for( uint16_t i = 0; i < mPosCoordszise; ++i )
    {
        console() << " mPosCoord " << mPosCoords[i] << std::endl;
    }

}


gl::VboMeshRef Blockletter01App::createPrimitivePyramidFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4, bool cavity )
{
    cinder::gl::VboMeshRef  mVboMeshPyramids;
    
    std::vector<uint16_t>   mIndicesPyramids;
    std::vector<ci::vec3>   mPosCoordsPyramids;
    std::vector<ci::vec3>   mNormalsPyramids;
    std::vector<ci::vec3>   mTangentsPyramids;
    std::vector<ci::vec2>   mTexCoordsPyramids;
    
    // Vertex
    //
    //     V2-----*         y
    //      :     |         |__x
    //   *-----V0 |       z/
    //      *.....V1
    //
    //  V3-----*
    
    // mUniteCube Index
    //      2-----6         y
    //      |     |         |__x
    //   1-----5  |       z/
    //      3.....7
    //
    //   0-----4
    
    // If subdivide
    mPosCoordsPyramids.push_back( point1 );
    mPosCoordsPyramids.push_back( point2 );
    mPosCoordsPyramids.push_back( point3 );
    mPosCoordsPyramids.push_back( point4 );
    
    // If we want to create cavities do not create the base triangle
    if ( cavity == false )
    {
        mIndicesPyramids.push_back( 1 );
        mIndicesPyramids.push_back( 2 );
        mIndicesPyramids.push_back( 0 );
    }
    mIndicesPyramids.push_back( 2 );
    mIndicesPyramids.push_back( 3 );
    mIndicesPyramids.push_back( 0 );
    
    mIndicesPyramids.push_back( 0 );
    mIndicesPyramids.push_back( 3 );
    mIndicesPyramids.push_back( 1 );
    
    mIndicesPyramids.push_back( 1 );
    mIndicesPyramids.push_back( 3 );
    mIndicesPyramids.push_back( 2 );
    
    // Use this normal for illumination
    // -> wireframe shader
    vec3 normal = computeTriangleNormal( point2, point3, point1 );
    vec3 gravityCenter = computeTriangleGravitycenter( point2, point3, point1 );
    mNormalsPyramids.push_back( gravityCenter + normal );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
     
    normal = computeTriangleNormal( point3, point4, point1 );
    gravityCenter = computeTriangleGravitycenter( point3, point4, point1 );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    
    normal = computeTriangleNormal( point1, point4, point2 );
    gravityCenter = computeTriangleGravitycenter( point4, point3, point2 );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    
    normal = computeTriangleNormal( point2, point4, point3 );
    gravityCenter = computeTriangleGravitycenter( point2, point4, point3 );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    mNormalsPyramids.push_back( gravityCenter + normal  );
    
    // Layout
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    
    mVboMeshPyramids = gl::VboMesh::create( mPosCoordsPyramids.size(), GL_TRIANGLES, { layout }, mIndicesPyramids.size() );
    mVboMeshPyramids->bufferAttrib( geom::POSITION, mPosCoordsPyramids.size() * sizeof( vec3 ), mPosCoordsPyramids.data() );
    mVboMeshPyramids->bufferAttrib( geom::NORMAL, mNormalsPyramids.size() * sizeof( vec3 ), mNormalsPyramids.data() );
    mVboMeshPyramids->bufferIndices( mIndicesPyramids.size() * sizeof( uint16_t ), mIndicesPyramids.data() );
    
    return mVboMeshPyramids;

}


vec3 Blockletter01App::computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 )
{
    return vec3( point1.x + point2.x + point3.x, point1.y + point2.y + point3.y, point1.z + point2.z + point3.z ) / 3.0f;
}


vec3 Blockletter01App::computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 )
{
    vec3 e0 = pos2 - pos1;
    vec3 e1 = pos3 - pos1;
    
    return normalize( cross( e0, e1 ));
}


void Blockletter01App::loadShaders()
{
    try {
        mTrailShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "trail.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Color shader shader: " << exc.what() );
    }
    
    try {
        mGodRaysShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "god_rays.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading GodRays shader: " << exc.what() );
    }
    
    try {
        mHdrShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "hdr.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading HDR shader: " << exc.what() );
    }
    
    try {
        mBloomShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "bloom.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Bloom shader: " << exc.what() );
    }
    
    try {
        mFxaaShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "fxaa.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Fxaa shader: " << exc.what() );
    }
    
    try {
        auto format = gl::GlslProg::Format()
        .vertex( loadAsset( "wireframe.vert" ) )
        .geometry( loadAsset( "wireframe.geom" ) )
        .fragment( loadAsset( "wireframe.frag" ) );
        
        mWireframeShader = gl::GlslProg::create( format );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading wireframe shader: " << exc.what() );
    }
    
    try {
        auto format = gl::GlslProg::Format()
        .vertex( loadAsset( "color.vert" ) )
        .geometry( loadAsset( "color.geom" ) )
        .fragment( loadAsset( "color.frag" ) );
        
        mColorShader = gl::GlslProg::create( format );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading color shader: " << exc.what() );
    }
}


void Blockletter01App::createGeometry()
{
    
    //mBatchSphere = gl::Batch::create( geom::Sphere().subdivisions( 40 ), mColorShader );
    mBatchSphere = gl::Batch::create( createPrimitivePyramidFromPoints( mUnitCube[5], mUnitCube[7], mUnitCube[2], mUnitCube[0],  false ), mColorShader );
    mBatchSphere->getGlslProg()->uniform( "uTex0", 0 );
    
    mBatchPyramid = gl::Batch::create( createPrimitivePyramidFromPoints( mUnitCube[5], mUnitCube[7], mUnitCube[2], mUnitCube[0],  false ), mWireframeShader );
    mBatchPyramid->getGlslProg()->uniform( "uTexNormal", 0 );
    mBatchPyramid->getGlslProg()->uniform( "uTexRoughness", 1 );
    
    gl::VboMeshRef  rect = gl::VboMesh::create( geom::Rect() );
    
    // Fade texture
    mBatchTrailRect = gl::Batch::create( rect, mTrailShader );
    mBatchTrailRect->getGlslProg()->uniform( "uBackground", 0 );
    mBatchTrailRect->getGlslProg()->uniform( "uForground", 1 );
    
    // GodRays Batch
    mBatchGodRays = gl::Batch::create( rect, mGodRaysShader );
    mBatchGodRays->getGlslProg()->uniform( "uTexFirstPass", 0 );
    
    // Bloom Batch
    mBatchBloomRect                 = gl::Batch::create( rect, mBloomShader );
    mBatchBloomRect->getGlslProg()->uniform( "uTexture", 0 );
    
    // HDR Batch
    mBatchHdrRect = gl::Batch::create( rect, mHdrShader );
    mBatchHdrRect->getGlslProg()->uniform( "uHdrBuffer1", 0 );
    mBatchHdrRect->getGlslProg()->uniform( "uHdrBuffer2", 1 );
    mBatchHdrRect->getGlslProg()->uniform( "uHdrBuffer3", 2 );
    
    // Fxaa Batch
    mBatchFxaaRect = gl::Batch::create( rect, mFxaaShader );
    mBatchFxaaRect->getGlslProg()->uniform(	"uSampler",	0 );
    
    // To screen Batch
    gl::GlslProgRef stockTexture    = gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchToScreenRect              = gl::Batch::create( rect, stockTexture );
}


void Blockletter01App::loadTextures()
{
    // Load the textures.
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    fmt.setWrap( GL_REPEAT, GL_REPEAT );
    mTextureStripes = gl::Texture::create( loadImage( loadAsset( "textures/stripes.jpg" ) ), fmt );
    mTextureNormal = gl::Texture::create( loadImage( loadAsset( "textures/normal-ogl.png" ) ), fmt );
    mTexRoughness = gl::Texture::create( loadImage( loadAsset( "textures/roughness.png" ) ), fmt );

}


// Save renderFbo
void Blockletter01App::saveImage()
{
    //console() << "mCurrentFrame : " << mCurrentFrame << std::endl;
    //console() << "mXMLcom_Medium.isLooping() : " << mXMLcom_Bass.isLooping() << std::endl;
    
    //for 30 fps
    /*if (  ( mLowfpsXML == true && mCurrentFrame % 2 == 0 && mXMLcom_Medium.isLooping() == true ) || ( mCurrentFrame >= 0 ) )
    {
       // console() << "mCurrentFrame: " << mCurrentFrame << std::endl;

        //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mFrameCounter ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
        mFrameCounter++;
    }
    else*/ if ( mXMLcom_Bass.isLooping() == 1 && mCurrentFrame > 968 )
    {
        //console() << "mCurrentFrame : " << mCurrentFrame << std::endl;
        // Pull down the current window as a surface and pass it to writeImage
        //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mCurrentFrame ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
    }
}


void Blockletter01App::createParams()
{
    mParams = params::InterfaceGl::create( getWindow(), "Geometry Demo", toPixels( ivec2( 300, 400 ) ) );
    mParams->setOptions( "", "valueswidth=100 refresh=0.1" );
    mParams->addParam( "Frame rate", &mFrameRate );
    mParams->addSeparator();
    mParams->addParam( "Pyramid Illum Size", &mPyramidsIlluminationSize, "min=-100.0 max= 100.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Brightness ", &mBrightness, "min=-1000.0 max= 1000.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Light Power ", &mLightPower, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Distance Power 1 ", &mDistancePower1, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Distance Power 2 ", &mDistancePower2, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "distance ligth", &mLightCamera.distanceRadius ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "distance conv Tet 1", &mDistanceConverstionTetraedre1 ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "distance conv Tet 2", &mDistanceConverstionTetraedre2 ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "Circle Size", &mCircleSize2 ).min( 1.5f ).max( 3.0f ).step( 0.01f);

    
    mParams->addSeparator();
    mParams->addParam( "Attenuation trail", &mTrailAttenuation, "min=0.0 max=5.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "Color 1 Red", &mColor1.x, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Color 1 Green", &mColor1.y, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Color 1 Blue", &mColor1.z, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Color 2 Red", &mColor2.x, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Color 2 Green", &mColor2.y, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Color 2 Blue", &mColor2.z, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addSeparator();
    mParams->addParam( "Exposure", &mExposure, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Decay", &mDecay, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Density", &mDensity, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addParam( "Weight", &mWeight, "min=0.0 max=5.0 step=0.01 keyIncr=a keyDecr=A" );
    mParams->addSeparator();
    mParams->addParam( "Bloomweight 0", &aBloomValues[0], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 1", &aBloomValues[1], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 2", &aBloomValues[2], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 3", &aBloomValues[3], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 3", &aBloomValues[4], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "HDR mix bloom GR", &mMixBloomGodrays, "min= 0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "HDR exposure", &mHdrExposure, "min= -1.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "HDR gamma", &mHdrGamma, "min= 0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Chromatic Aberration", &mChromaticAberration, "min=0.0 max=2.0 step=0.0001 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "Axis X Rot speed", &mReactiveCam.mCamParams.mAxisXRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Axis Y Rot speed", &mReactiveCam.mCamParams.mAxisYRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotation speed", &mReactiveCam.mCamParams.mRotationSpeed, "min=-1.0 max=1.0 step=0.001 keyIncr=q keyDecr=w" );
}


void Blockletter01App::setXMLAnimationPhases()
{
    if( mXMLcom_Bass.mIsBeat0 == true )
    {
        mbeatCounter_1 = 0.0f;
        mbeatCounter_2 = 0.0f;
        mIsBeat = true;
    }
    
    float ease = 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) );
    if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.02f;
    
    float ease2 = easeOutExpo( clamp( mbeatCounter_2, 0.0f, 1.0f ) );
    if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02f;
    
    float sizeCoef_Min = 0.0;
    float sizeCoef_Max = 0.18;
    float lin1 = linearize( 0.0f, 1.0f, sizeCoef_Min, sizeCoef_Max, ease );
    mDistancePower1 = -0.35 - lin1;
    
    float circleCoef_Min = 1.60;
    float circleCoef_Max = 2.15;
    float lin2 = linearize( 0.0f, 1.0f, circleCoef_Min, circleCoef_Max, ease2 );
    mCircleSize = lin2;
    
    if( mXMLcom_Medium.mIsBeat1 == true )
    {
        mbeatCounter_3 = 0.0f;
        //mIsBeat2 = true;
    }

    float ease3 = easeOutExpo( clamp( mbeatCounter_3, 0.0f, 1.0f ) );
    if ( mbeatCounter_3 < 1.0f ) mbeatCounter_3 += 0.02f;
    
    float ease4 = 1.0 - easeOutExpo( clamp( mbeatCounter_3, 0.0f, 1.0f ) );
    if ( mbeatCounter_3 < 1.0f ) mbeatCounter_3 += 0.02f;
    
    float sizeCoef_Min2 = 2.5;
    float sizeCoef_Max2 = 2.0;
    float lin3 = linearize( 0.0f, 1.0f, sizeCoef_Min2, sizeCoef_Max2, ease3 );
    mCircleSize2 = lin3;
    //console() << "lin3: " << lin3 << std::endl;
    
    sizeCoef_Min = 0.0;
    sizeCoef_Max = 0.05;
    float lin4 = linearize( 0.0f, 1.0f, sizeCoef_Min, sizeCoef_Max, ease4 );
    mDistancePower2 = -0.23 + lin4;
}


void Blockletter01App::transition_0( bool start )
{
    if ( start == true )
    {
    }
}


void Blockletter01App::transition_1( bool start )
{
    if ( start == true )
    {
    }
}


void Blockletter01App::transition_2( bool start )
{
    if ( start == true )
    {
    }
}

void Blockletter01App::mouseDown( MouseEvent event )
{
    //mIsBeat = true;
    mIsBeat2 = true;

}


void Blockletter01App::mouseDrag( MouseEvent event )
{
    
}


void Blockletter01App::keyDown( KeyEvent event )
{
    
}


// conv lin from [a0-a1] to [b0-b1]
// a0-a1 -> b0-b1 - ex: 0.0-2.0 -> 0.0-1.0
// out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
float Blockletter01App::linearize( float a0, float a1, float b0, float b1, float in )
{
    return ( ( in - a0 ) / ( a1 - a0 ) ) * ( b1 - b0 ) + b0;
}


vec3 Blockletter01App::rotateAxis( bool beat, vec3 &rotationVectorFrom, vec3 &rotationVectorTo, float easeSpeed )
{
    if ( beat == true )
    {
        mbeatCounter = 0.0f;
        rotationVectorFrom = rotationVectorTo;
        vec3 rotationVectorTmp = normalize( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
        rotationVectorTo = normalize( cross( rotationVectorFrom, rotationVectorTmp ) );
    }
    
    float ease = easeOutExpo( clamp( mbeatCounter, 0.0f, 1.0f ) );
    if ( mbeatCounter < 1.0f ) mbeatCounter += easeSpeed;
    
    return slerp( rotationVectorFrom, rotationVectorTo, ease );
}


// Vector slerp
vec3 Blockletter01App::slerp( vec3 &start, vec3 &end, float percent )
{
    // From cinder 8.3 vector.slerp()
    // Dot product - the cosine of the angle between 2 vectors.
    float cosAlpha = dot( start, end );
    
    clamp( cosAlpha, -1.0f, 1.0f );
    
    float alpha = acos( cosAlpha );
    
    float sinAlpha = sin( alpha );
    
    float t1 = sin( ( 1.0 - percent ) * alpha ) / sinAlpha;
    float t2 = sin( percent * alpha ) / sinAlpha;
    
    return start * t1 + end * t2;
    
    // or from internet
    // Special Thanks to Johnathan, Shaun and Geof!
    /*Vector3 Slerp(Vector3 start, Vector3 end, float percent)
     {
     // Dot product - the cosine of the angle between 2 vectors.
     float dot = Vector3.Dot(start, end);
     // Clamp it to be in the range of Acos()
     // This may be unnecessary, but floating point
     // precision can be a fickle mistress.
     Mathf.Clamp(dot, -1.0f, 1.0f);
     // Acos(dot) returns the angle between start and end,
     // And multiplying that by percent returns the angle between
     // start and the final result.
     float theta = Mathf.Acos(dot)*percent;
     Vector3 RelativeVec = end - start*dot;
     RelativeVec.Normalize();     // Orthonormal basis
     // The final result.
     return ((start*Mathf.Cos(theta)) + (RelativeVec*Mathf.Sin(theta)));
     }*/
}


// Rotate an object fron angle d and vector z => Inigo Quilez
mat4 Blockletter01App::rotationAlign( vec3 d, vec3 z )
{
    vec3  v = cross( z, d );
    float c = dot( z, d );
    float k = 1.0/(1.0+c);
    
    mat4 aMat4 = mat4( v.x*v.x*k + c,       v.x*v.y*k + v.z,    v.x*v.z*k - v.y,    0.0f,
                      v.y*v.x*k - v.z,      v.y*v.y*k + c,      v.y*v.z*k + v.x,    0.0f,
                      v.z*v.x*k + v.y,      v.z*v.y*k - v.x,    v.z*v.z*k + c,      0.0f,
                      0.0f,                 0.0f,               0.0f,               1.0f);
    return aMat4;
}




CINDER_APP( Blockletter01App, RendererGl( RendererGl::Options() ), prepareSettings )
