
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"

#include "DeformedSphere.h"
#include "TF_Particles.h"
#include "Model.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using std::vector;

#undef PHI	// take the reciprocal of phi, to obtain an icosahedron that fits a unit cube
#define PHI (1.0f / ((1.0f + math<float>::sqrt(5.0f)) / 2.0f))


class deformedSphereMappingV2App : public App {
public:
    deformedSphereMappingV2App();
    static void prepareSettings( Settings *settings );
    void    keyDown( KeyEvent event ) override;
    void	setup() override;
    void	resize() override;
    void	update() override;
    void	draw() override;
    void	createBatches();
    
    gl::BatchRef                mBatch, mBatchBloomBlurRect, mBatchBloomCompositeRect, mBatchStockTextureRect, mBatchColorRect, mBatchBloomHighpassRect, mBatchFxaaRect;
    gl::TextureRef              mDiffuseTex, mNormalTex, mStripeTex, mSpecularTex;
    gl::GlslProgRef             mGlsl;
    
    gl::FboRef                  mRenderOpbjetcDepth, mRenderFBO, mFboTrail, mFboAccum, mFboComposite;
    ci::gl::Texture2dRef		mTexturemRenderFBO[ 3 ];
    ci::gl::Texture2dRef		mTextureFboAccum[ 3];
    ci::gl::Texture2dRef		mTextureFboTrail[ 4 ];
    ci::gl::Texture2dRef		mTextureFboComposite[ 2 ];
    
    
    // Deformed sphere
    DeformedSphere              mDeformedSphere;
    ci::gl::VboRef				mVboInstancedDeformedSphere;
    ci::gl::BatchRef			mBatchGBufferdeformedSphere;
    quat                        mQuat;
    float                       mCounter1 = 0.0f;
    float                       mCounter2 = 0.0f;
    
    float                       mColorR = 0.45f;
    float                       mColorG = 0.34f;
    float                       mColorB = 0.85f;
    
    float                       mColorR2 = 0.45f;
    float                       mColorG2 = 0.34f;
    float                       mColorB2 = 0.85f;
    
    float                       mkExposure = 1.00f;
    float                       mkGamma = 1.00f;
    float                       mkChromaticAberration = 0.0011f;
    
    // Sphere PARAM ////////////////////////////////////////////////////
    params::InterfaceGlRef      mParams;
    float                       mCompositeAttenuation = 1.50f;
    float                       mBloomScaleCoef = 6.00f;
    float                       mTrailAttenuation = 0.020f;
    float                       mCompositeTheta = 0.50f;
    float                       mBloomAttenuation = 2.80f;
    float                       mSpecularAttenuation = 0.65f;
    float                       mDiffuseStripesAttenuation = 2.80;
    float                       mParticlesBloomLuminosity = 1.00f;
    
    // FBO AND PING PONG ///////////////////////////////////////////
    size_t                      ping = 0;
    size_t                      pong = 1;

    // Camera
    CameraPersp                 mCam;
    vec3                        mCamLastPosition;
    bool                        mIsFirt = true;
    vec3                        mCameraSlerpA;
    vec3                        mCameraSlerpB = vec3( 0.0, 0.0, 0.0 );
    
    // PARTICLES ////////////////////////////////////////////////
    TF_Particles                mTF_Partices;
    
    // FXAA /////////////////////////////////////////////////////
    bool                        mEnabledFxaa = true;
};


deformedSphereMappingV2App::deformedSphereMappingV2App()
{
    // Load shaders and create batches
    createBatches();
    
    // Call resize to create FBOs
    resize();
}


void deformedSphereMappingV2App::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 60 );
    settings->setWindowSize( 1280, 570 );
#if defined( CINDER_MSW )
    settings->setConsoleWindowEnabled();
#endif
    settings->setMultiTouchEnabled( false );
}


void deformedSphereMappingV2App::keyDown( KeyEvent event )
{
    console() << "keyDown: " << event.getChar() <<  std::endl;
    if( event.getChar() == 'f' ) {
        // Toggle full screen.
        resize();
        setFullScreen( !isFullScreen() );
    }
    else if( event.getChar() == 'x' ) {
        // fxxa
        console() << "fxaa: " << mEnabledFxaa << std::endl;
        mEnabledFxaa = !mEnabledFxaa;
    }
}


void deformedSphereMappingV2App::createBatches()
{
    // Shortcut for shader loading and error handling
    auto loadGlslProg = [ & ]( const gl::GlslProg::Format& format ) -> gl::GlslProgRef
    {
        string names = format.getVertexPath().string() + " + " +
        format.getFragmentPath().string();
        gl::GlslProgRef glslProg;
        try {
            glslProg = gl::GlslProg::create( format );
        } catch ( const Exception& ex ) {
            CI_LOG_EXCEPTION( names, ex );
            quit();
        }
        return glslProg;
    };
    
    // Textures
    mDiffuseTex = gl::Texture::create( loadImage( loadAsset( "sphere/diffuseMap.jpg" ) ), gl::Texture::Format().mipmap() );
    mDiffuseTex->setWrap( GL_REPEAT, GL_REPEAT );
    mNormalTex = gl::Texture::create( loadImage( loadAsset( "sphere/normalMap.jpg" ) ), gl::Texture::Format().mipmap() );
    mNormalTex->setWrap( GL_REPEAT, GL_REPEAT );
    mStripeTex = gl::Texture::create( loadImage( loadAsset( "sphere/stripe.png" ) ), gl::Texture::Format().mipmap() );
    mStripeTex->setWrap( GL_REPEAT, GL_REPEAT );
    mSpecularTex = gl::Texture::create( loadImage( loadAsset( "sphere/specular.png" ) ), gl::Texture::Format().mipmap() );
    mSpecularTex->setWrap( GL_REPEAT, GL_REPEAT );
    
    // Deformed Sphere
    mGlsl = gl::GlslProg::create( loadAsset( "sphere/shader.vert" ), loadAsset( "sphere/shader.frag" ) );
    
    mBatch = gl::Batch::create( mDeformedSphere.create(), mGlsl );
    
    gl::ScopedGlslProg glslScp( mGlsl );
    mGlsl->uniform( "uDiffuseMap", 4 );
    mGlsl->uniform( "uNormalMap", 1 );
    mGlsl->uniform( "uStripeMap", 2 );
    mGlsl->uniform( "uSpecular", 3 );
    mGlsl->uniform( "uLightLocViewSpace", vec3( 0, 0, 1 ) );
    
    // Create other batchs
    int32_t version					= 330;
    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );
    
    DataSourceRef fragBloomBlur		= loadAsset( "sphere/blur.frag" );
    DataSourceRef vertPassThrough	= loadAsset( "sphere/pass_through.vert" );
    gl::GlslProgRef bloomBlur       = loadGlslProg( gl::GlslProg::Format().version( version )
                                                   .vertex( vertPassThrough ).fragment( fragBloomBlur ));
    
    mBatchBloomBlurRect             = gl::Batch::create( rect, bloomBlur );
    mBatchBloomBlurRect->getGlslProg()->uniform( "sphere/uSampler", 0 );
    
    // High Pass
    DataSourceRef fragHighpass		= loadAsset( "sphere/highpass.frag" );
    gl::GlslProgRef bloomHighpass       = loadGlslProg( gl::GlslProg::Format().version( version )
                                                       .vertex( vertPassThrough ).fragment( fragHighpass ));
    
    mBatchBloomHighpassRect             = gl::Batch::create( rect, bloomHighpass );
    mBatchBloomHighpassRect->getGlslProg()->uniform( "uSampler", 0 );
    
    
    // final render
    const vec2 sz = toPixels( getWindowSize() );
    DataSourceRef fragBloomComposite    = loadAsset( "sphere/composite.frag" );
    gl::GlslProgRef bloomComposite      = loadGlslProg( gl::GlslProg::Format().version( version )
                                                       .vertex( vertPassThrough ).fragment( fragBloomComposite ));
    mBatchBloomCompositeRect            = gl::Batch::create( rect, bloomComposite );
    mBatchBloomCompositeRect->getGlslProg()->uniform(	"uSamplerTextureColor",		0 );
    mBatchBloomCompositeRect->getGlslProg()->uniform(	"uSamplerBloom",            1 );
    mBatchBloomCompositeRect->getGlslProg()->uniform(	"uSamplerTextureTrail",     2 );
    mBatchBloomCompositeRect->getGlslProg()->uniform(	"uPixel", vec2( 1.0f ) / vec2( sz ) );
    
    // To screen
    gl::GlslProgRef stockTexture	= gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchStockTextureRect			= gl::Batch::create( rect, stockTexture );
    
    // Fade texture
    DataSourceRef fragColor		= loadAsset( "sphere/color.frag" );
    gl::GlslProgRef colorGlsl   = loadGlslProg( gl::GlslProg::Format().version( version )
                                               .vertex( vertPassThrough ).fragment( fragColor ) );
    mBatchColorRect             = gl::Batch::create( rect, colorGlsl );
    mBatchColorRect->getGlslProg()->uniform( "uBackground", 0 );
    mBatchColorRect->getGlslProg()->uniform( "uForground", 1 );
    
    // Fxaa
    DataSourceRef fragPostFxaa	= loadAsset( "post/fxaa.frag" );
    gl::GlslProgRef postFxaa	= loadGlslProg( gl::GlslProg::Format().version( version )
                                               .vertex( vertPassThrough ).fragment( fragPostFxaa ) );
    mBatchFxaaRect              = gl::Batch::create( rect, postFxaa );
    mBatchFxaaRect->getGlslProg()->uniform(	"uSampler",	0 );
    
    
    geom::BufferLayout bufferLayout;
    size_t stride = sizeof( Model );
    bufferLayout.append( geom::Attrib::CUSTOM_0, 16, stride, 0, 1 );
    bufferLayout.append( geom::Attrib::CUSTOM_1, 9, stride, sizeof( mat4 ), 1 );
    vector<Model> deformedSpheres( 1 );
    mVboInstancedDeformedSphere	= gl::Vbo::create( GL_ARRAY_BUFFER, deformedSpheres.size() * stride, deformedSpheres.data(), GL_DYNAMIC_DRAW );
    gl::VboMeshRef deformedSphere = mDeformedSphere.create();
    deformedSphere->appendVbo( bufferLayout, mVboInstancedDeformedSphere );
    
    // Create instanced batches
    mBatchGBufferdeformedSphere = gl::Batch::create( deformedSphere, mGlsl, {
        { geom::Attrib::CUSTOM_0, "vInstanceModelMatrix" },
        { geom::Attrib::CUSTOM_1, "vInstanceNormalMatrix" }
    } );
    
}

void deformedSphereMappingV2App::resize()
{
    //hideCursor();
    
    ivec2 winSize	= toPixels( getWindowSize() );
    int32_t h		= winSize.y;
    int32_t w		= winSize.x;

    mCam.setPerspective( 60, w/h, 0.1, 10 );


    
    mCam.setAspectRatio( getWindowAspectRatio() );

    gl::setMatrices( mCam );
    
    // Texture format for depth buffers
    // Attach only depth texture
    {
        gl::Texture2d::Format depthTextureFormat = gl::Texture2d::Format()
        .internalFormat( GL_DEPTH_COMPONENT32F )
        .magFilter( GL_LINEAR )
        .minFilter( GL_LINEAR )
        .wrap( GL_CLAMP_TO_EDGE )
        .dataType( GL_FLOAT );
        
        ivec2 sz = ivec2( w, h );
        gl::Fbo::Format fboFormat;
        
        fboFormat.depthTexture( depthTextureFormat );
        mRenderOpbjetcDepth = gl::Fbo::create( sz.x, sz.y, fboFormat );
        
        const gl::ScopedFramebuffer scopedFramebuffer( mRenderOpbjetcDepth );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderOpbjetcDepth->getSize() );
        gl::clear();
    }
    
    
    // Set up the buffer for rendering
    // 0 GL_COLOR_ATTACHMENT0	trail
    // 1 GL_COLOR_ATTACHMENT1	scene
    // 2 GL_COLOR_ATTACHMENT1	scene for bloom
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 3; ++i ) {
            mTexturemRenderFBO[ i ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                            .internalFormat( GL_RGBA16F_ARB )
                                                            .magFilter( GL_LINEAR )
                                                            .minFilter( GL_LINEAR )
                                                            .wrap( GL_CLAMP_TO_EDGE )
                                                            .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTexturemRenderFBO[ i ] );
        }
        
        mRenderFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        {
            const gl::ScopedFramebuffer scopedFramebuffer( mRenderFBO );
            const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
            gl::clear();
        }
    }
    
    // Ping Pong Trails Trail
    {
        ivec2 sz = ivec2( w, h );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 4; ++i ) {
            mTextureFboTrail[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                          .internalFormat( GL_RGBA16F_ARB )
                                                          .magFilter( GL_LINEAR )
                                                          .minFilter( GL_LINEAR )
                                                          .wrap( GL_CLAMP_TO_EDGE )
                                                          .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureFboTrail[ i ] );
        }
        mFboTrail = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mFboTrail );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboTrail->getSize() );
        gl::clear();
    }
    
    // Light accumulation frame buffer
    // 0 GL_COLOR_ATTACHMENT0 Light accumulation
    // 1 GL_COLOR_ATTACHMENT1 Bloom ping
    // 2 GL_COLOR_ATTACHMENT2 Bloom pong
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 3; ++i ) {
            mTextureFboAccum[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                          .internalFormat( GL_RGBA16F_ARB )
                                                          //.internalFormat( GL_RGBA ) effet special rayure mais scopedBlendAlpha doit etre desactive
                                                          .magFilter( GL_LINEAR )
                                                          .minFilter( GL_LINEAR )
                                                          .wrap( GL_CLAMP_TO_EDGE )
                                                          .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureFboAccum[ i ] );
        }
        mFboAccum = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mFboAccum );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboAccum->getSize() );
        gl::clear();
    }
    
    // Composite
    {
        ivec2 sz = ivec2( w, h );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 2; ++i ) {
            mTextureFboComposite[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                              .internalFormat( GL_RGBA16F_ARB )
                                                              //.internalFormat( GL_RGBA ) effet special rayure mais scopedBlendAlpha doit etre desactive
                                                              .magFilter( GL_LINEAR )
                                                              .minFilter( GL_LINEAR )
                                                              .wrap( GL_CLAMP_TO_EDGE )
                                                              .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureFboComposite[ i ] );
        }
        mFboComposite = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mFboComposite );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboComposite->getSize() );
        gl::clear();
    }
}

void deformedSphereMappingV2App::setup()
{
    mCam = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f );
    mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0, 1000 );
    mCam.lookAt( vec3(5.0f, 0.0f, 0.0f), vec3( 0, 0, 0 ) );
    mCamLastPosition = vec3(5.0f, 0.0f, 0.0f);
    
    // Particcles
    mTF_Partices.setup();
    
    mParams = params::InterfaceGl::create( getWindow(), "Perlin settings", toPixels( ivec2( 230, 220 ) ) );
    mParams->addParam( "Attenuation trail", &mTrailAttenuation, "min=0.0 max=5.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Attenuation Composite", &mCompositeAttenuation, "min=0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Attenuation Bloom", &mBloomAttenuation, "min=0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloom Scale", &mBloomScaleCoef, "min=1.0 max=20.0 step=0.1 keyIncr=q keyDecr=w" );
    mParams->addParam( "Particles Bloom coef", &mParticlesBloomLuminosity, "min=0.0 max=5.0 step=0.1 keyIncr=q keyDecr=w" );
    mParams->addParam( "Theta", &mCompositeTheta, "min=0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    
    mParams->addSeparator();
    mParams->addParam( "Color R", &mColorR, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color G", &mColorG, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color B", &mColorB, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Stripes Attenuation", &mDiffuseStripesAttenuation, "min=0.0 max=15.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color2 R", &mColorR2, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color2 G", &mColorG2, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color2 B", &mColorB2, "min=0.0 max=1.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color2 Attenuation", &mSpecularAttenuation, "min=0.0 max=10.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    
    
    mParams->addParam( "kChromaticAberration", &mkChromaticAberration, "min=0.0 max=2.0 step=0.0001 keyIncr=q keyDecr=w" );
    mParams->addParam( "kExposure", &mkExposure, "min=0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "kGamma", &mkGamma, "min=0.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    
    // params
    mParams->addSeparator();
    mParams->addParam( "NoiseScale", &mTF_Partices.mNoiseScale, "min=-10.0 max=10.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Noise_strength", &mTF_Partices.mNoise_strength, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Length_scale", &mTF_Partices.mLength_scale, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Progression_rate", &mTF_Partices.progression_rate, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotational_strength", &mTF_Partices.rotational_strength, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotational_speed_strength", &mTF_Partices.rotational_speed_strength, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotational_speed_strength2", &mTF_Partices.rotational_speed_strength2, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Centriped attraction", &mTF_Partices.centripedAttraction, "min=-20.0 max=20.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "VelocityDamping", &mTF_Partices.velocityDamping, "min=0.0 max=2.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "AdjustedZFar", &mTF_Partices.adjustedZFar, "min=0.0 max=1000.0 step=1.0 keyIncr=q keyDecr=w" );
    
    mQuat = quat();
}


void deformedSphereMappingV2App::update()
{
    float distmin = length(vec3( 0.0f, 0.0f, 5.0f ));
    float distmax1 = length(vec3( 0.0f, -6.0f, 6.0f ));
    
    // conv lin a to b
    // a0-a1 -> b0-b1 : 0.02-0.7 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // (fLifeTime - 0) -> (0 - 1)
    vec3 dist = vec3( 0.0f , -cos(mCounter1*0.10)*3.0f, 4.5f + sin(mCounter1*0.15)*2.0f );
    float rotCoef = clamp( ((length(dist) - distmin) / (distmax1 - distmin)) * (1.0f - 0.7f) + 0.7f, 0.7f, 1.0f);
    float distToCenter = length( dist );
    
    if( mIsFirt)
    {
        mCameraSlerpA = normalize( -dist + vec3( 0.0f , -cos(( mCounter1 - 0.01667f ) * 0.10 )*6.0f, 5.0f + sin(( mCounter1 - 0.01667f )*0.15 ) * 2.0f ));
        mIsFirt = false;
    }
    
    mCamLastPosition = mCam.getEyePoint();
    vec3 camLookAt = normalize( dist - mCamLastPosition );
    vec3 camLookAtCenter = normalize( -dist );
    
    if ( length( mCamLastPosition) > distToCenter )
    {
        camLookAt = normalize( mCamLastPosition - dist );
    }
    
    vec3 slerped = slerp( mCameraSlerpA, camLookAtCenter, 0.2f ); // 0.8
    
    if ( isnan(slerped.x) == true || isnan(slerped.y) == true ||isnan(slerped.z) == true )
    {
        // NAN,than keep the last value
        slerped = mCameraSlerpA;
    }
    else{
        mCameraSlerpA = slerped;
    }
    
    mCam.lookAt( dist, normalize(slerped) * distToCenter  );
    
    // Particcles
    mTF_Partices.update(mCounter1);
    
    // Update deformedSphere
    {
        glm::mat4 trans = glm::mat4(1.0f);

        trans = glm::rotate(trans, glm::radians((float) mCounter2 ), glm::vec3(0.0f, 1.0f, 0.0f));
        
        Model* deformedSphere = (Model*)mVboInstancedDeformedSphere->map( GL_READ_WRITE );
        
        // should not use instancied objects only for 1 object
        for ( int32_t i = 0; i < 1; ++i ) {
            mat4 m( 1.0f );
            m = trans * m;
            deformedSphere->setModelMatrix( m );
            deformedSphere->setNormalMatrix( glm::inverseTranspose( mat3( mCam.getViewMatrix() * m ) ) );
            ++deformedSphere;
        }
        mVboInstancedDeformedSphere->unmap();
    }
    // 60 fps
    mCounter1 += 0.01667f * rotCoef + 0.01f;
    mCounter2 += 0.8f * rotCoef;
    
}


void deformedSphereMappingV2App::draw()
{
    ivec2 winSize	= toPixels( getWindowSize() );
    int32_t h		= winSize.y;
    int32_t w		= winSize.x;

    
    // Draw object texture only in depth texture
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderOpbjetcDepth );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderOpbjetcDepth->getSize() );
        gl::clear();
        gl::setMatrices( mCam );
        const gl::ScopedMatrices scopedMatrices;
        const gl::ScopedFaceCulling scopedFaceCulling( true, GL_BACK );
        
        // turn on z-buffering
        gl::enableDepthRead();
        gl::enableDepthWrite();
        
        // Disable color.
        gl::colorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        mBatchGBufferdeformedSphere->drawInstanced( 1 );
    }
    
    gl::colorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    
    // Clear AO buffer whether we use it or not
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        {
            const static GLenum buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
            };
            gl::drawBuffers( 3, buffers );
            
        }
        gl::clear();
        gl::setMatrices( mCam );
        gl::ScopedModelMatrix modelScope;
        const gl::ScopedMatrices scopedMatrices;
        const gl::ScopedFaceCulling scopedFaceCulling( true, GL_BACK );
        {
            gl::enableDepthWrite();
            gl::enableDepthRead();
            mNormalTex->bind( 1 );
            mStripeTex->bind( 2 );
            mSpecularTex->bind( 3 );
            mDiffuseTex->bind(4);
            mGlsl->uniform( "Time", vec2( mCounter1*0.022f, 0.0f));
            mGlsl->uniform( "uColorR", mColorR );
            mGlsl->uniform( "uColorG", mColorG );
            mGlsl->uniform( "uColorB", mColorB );
            mGlsl->uniform( "uDiffuseStripesAttenuation", mDiffuseStripesAttenuation );
            mGlsl->uniform( "uColorR2", mColorR2 );
            mGlsl->uniform( "uColorG2", mColorG2);
            mGlsl->uniform( "uColorB2", mColorB2 );
            mGlsl->uniform( "uSpecularAttenuation", mSpecularAttenuation );
            
            //mBatch->draw();
            mBatchGBufferdeformedSphere->drawInstanced( 1 );
        }
        
        // Particles
        mTF_Partices.draw( mCam, 1.0f, mParticlesBloomLuminosity, vec2( 1.0f / (float)w,  1.0f / (float)h), mRenderOpbjetcDepth->getDepthTexture() );
    }
    
    // Blur sceme
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mFboAccum );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboAccum->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        
        gl::setMatricesWindow( mFboAccum->getSize() );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( (float)mFboAccum->getWidth()/2.0f, (float)mFboAccum->getHeight()/2.0f );
        gl::scale( mFboAccum->getWidth(), mFboAccum->getHeight() );
        
        // Run a horizontal blur pass
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
            gl::clear();
            mBatchBloomBlurRect->getGlslProg()->uniform( "uAxis", vec2( 1.0f, 0.0f ) );
            mBatchBloomBlurRect->getGlslProg()->uniform( "InvViewportSize", 3.0f * vec2( 1.0f / (float)w,  1.0f / (float)h));
            const gl::ScopedTextureBind scopedTextureBind( mTexturemRenderFBO[ 2 ], 0 );
            mBatchBloomBlurRect->draw();
        }
        
        // Run a vertical blur pass
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT2 );
            gl::clear();
            mBatchBloomBlurRect->getGlslProg()->uniform( "uAxis", vec2( 0.0f, 1.0f ) );
            mBatchBloomBlurRect->getGlslProg()->uniform( "InvViewportSize", 3.0f * vec2( 1.0f / (float)w,  1.0f / (float)h));
            const gl::ScopedTextureBind scopedTextureBind( mTextureFboAccum[ 1 ], 0 );
            mBatchBloomBlurRect->draw();
        }
    }
    
    // Trails + trail blur
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mFboTrail );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0  + (GLenum)ping );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboAccum->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mFboTrail->getSize() );
        //gl::clear();
        
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( (float)mFboAccum->getWidth()/2.0f, (float)mFboAccum->getHeight()/2.0f );
        gl::scale( mFboAccum->getWidth(), mFboAccum->getHeight() );
        
        // Dim the light accumulation buffer to produce trails. Lower alpha
        // makes longer trails and paint light sources.
        {
            const gl::ScopedTextureBind scopedTextureBind0( mTextureFboTrail[ pong ], 0 );
            const gl::ScopedTextureBind scopedTextureBind1( mTexturemRenderFBO[ 0 ], 1 );
            mBatchColorRect->getGlslProg()->uniform( "attenuation", mTrailAttenuation );
            mBatchColorRect->draw();
        }
        
        // Run a horizontal blur pass
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT2 );
            mBatchBloomBlurRect->getGlslProg()->uniform( "uAxis", vec2( 1.0f, 0.0f ) );
            mBatchBloomBlurRect->getGlslProg()->uniform( "InvViewportSize",  mBloomScaleCoef * vec2( 1.0f / (float)w,  1.0f / (float)h));
            const gl::ScopedTextureBind scopedTextureBind( mTextureFboTrail[ ping ], 0 );
            mBatchBloomBlurRect->draw();
        }
        
        // Run a vertical blur pass
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT3 );
            gl::clear();
            mBatchBloomBlurRect->getGlslProg()->uniform( "uAxis", vec2( 0.0f, 1.0f ) );
            mBatchBloomBlurRect->getGlslProg()->uniform( "InvViewportSize",  mBloomScaleCoef * vec2( 1.0f / (float)w,  1.0f / (float)h));
            const gl::ScopedTextureBind scopedTextureBind( mTextureFboTrail[ 2 ], 0 );
            mBatchBloomBlurRect->draw();
        }
    }
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mFboComposite );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboComposite->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mFboComposite->getSize() );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( mFboComposite->getSize() / 2 );
        gl::scale( mFboComposite->getSize() );
        
        if ( mEnabledFxaa )
        {
            {
                gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
                const gl::ScopedTextureBind scopedTextureBind0( mTexturemRenderFBO[ 1 ], 0 ); // texture
                const gl::ScopedTextureBind scopedTextureBind2( mTextureFboAccum[ 2 ], 1 ); // texture bloom
                const gl::ScopedTextureBind scopedTextureBind1( mTextureFboTrail[ 3 ], 2 ); // trails
                
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kAttenuation", mCompositeAttenuation );
                
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kAttenuation2", mBloomAttenuation );
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kChromaticAberration", mkChromaticAberration );
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kTheta", mCompositeTheta );
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kExposure", mkExposure );
                mBatchBloomCompositeRect->getGlslProg()->uniform( "kGamma", mkGamma );
                mBatchBloomCompositeRect->draw();
            }
            {
                // To keep bandwidth in check, we aren't using any hardware
                // anti-aliasing (MSAA). Instead, we use FXAA as a post-process
                // to clean up our image.
                gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
                const gl::ScopedTextureBind scopedTextureBind0( mTextureFboComposite[ 0 ], 0 ); // texture
                mBatchFxaaRect->getGlslProg()->uniform( "uPixel", 1.0f / vec2( mFboComposite->getSize() ) );
                mBatchFxaaRect->draw();
            }
        }
        else
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
            const gl::ScopedTextureBind scopedTextureBind0( mTexturemRenderFBO[ 1 ], 0 ); // texture
            const gl::ScopedTextureBind scopedTextureBind2( mTextureFboAccum[ 2 ], 1 ); // texture bloom
            const gl::ScopedTextureBind scopedTextureBind1( mTextureFboTrail[ 3 ], 2 ); // trails
            
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kAttenuation", mCompositeAttenuation );
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kAttenuation2", mBloomAttenuation );
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kChromaticAberration", mkChromaticAberration );
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kTheta", mCompositeTheta );
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kExposure", mkExposure );
            mBatchBloomCompositeRect->getGlslProg()->uniform( "kGamma", mkGamma );

            mBatchBloomCompositeRect->draw();
        }
    }
    {
        // Render our final image to the screen
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), toPixels( getWindowSize() ) );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( toPixels( getWindowSize() ) );
        gl::translate( toPixels( getWindowSize() / 2 )  );
        gl::scale( toPixels( getWindowSize() ) );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        const gl::ScopedTextureBind scopedTextureBind0( mTextureFboComposite[ 1 ], 0 );
        mBatchStockTextureRect->draw();
    }
    
    ping = pong;
    pong = ( ping + 1 ) % 2;
    
    mParams->draw();
}


CINDER_APP( deformedSphereMappingV2App, RendererGl( RendererGl::Options().version( 3, 3 )), &deformedSphereMappingV2App::prepareSettings )
