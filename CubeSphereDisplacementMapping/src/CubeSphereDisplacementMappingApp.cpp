#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"


#include "CubeSphere.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CubeSphereDisplacementMappingApp : public App {
public:
    void setup() override;
    void resize() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void saveImage();
    
    CameraPersp             mCam;
    mat4                    mCubeRotation;
    vec3                    mLightPosWorldSpace;
    bool                    mChange = false;
    float                   mDisplacementCoef = 0.0f;
    
    gl::BatchRef            mBatch, mBatchBloomRect, mBatchFinalRenderRect, mBatchFxaaRect, mBatchToScreenRect;
    gl::GlslProgRef         mGlsl;
    
    // FBO
    gl::FboRef              mRenderFBO, mBloomFBO;
    ci::gl::Texture2dRef	mTexturemRender[ 5 ];
    ci::gl::Texture2dRef	mTextureBloom[ 2 ];
    vec2                    mFboSize = vec2 (1920, 1080);
    
    // SAVE IMAGE ///////////////////////////////////////////////
    int                     mCurrentFrame = 0;
    bool                    mSaveScreen = false;
    
    // HDR
    float                   mExposure = 3.61f;
    float                   mGamma = 0.44f;
    float                   mLight = 0.0f;
    
    // BLOOM
    // ref basic bloom : "LearnOpenGL - Bloom"
    // ref optimized linear sampling "Efficient blur with linear sampling" in the app folder
    float                       aBaseweight[5];
    float                       aBloomweight[ 3 ];
    float                       aBaseoffset[5];
    float                       aBloomoffset[ 3 ];
    
    // PARAM
    params::InterfaceGlRef  mParams;
    float mCounter = 0.0f;
    
    // Deformed sphere
    CubeSphere              mCubeSphere;
    gl::TextureCubeMapRef   mHeightTex, mOccTex, mTranslucencyTex, mReflexionTex;
    
    // TESSALATION
    float                   mInnerLevel, mOuterLevel;
    fs::path                glDir = "spheresmaps";
    
};

void CubeSphereDisplacementMappingApp::setup()
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
    
    // Bloom Weights
    // aBaseweight represents weights for not optimized linear sampling
    aBaseweight[0] = 0.2270270270f;
    aBaseweight[1] = 0.1945945946f;
    aBaseweight[2] = 0.1216216216f;
    aBaseweight[3] = 0.0540540541f;
    aBaseweight[4] = 0.0162162162f;
    
    // Here we calculate optimized linear sampling weights
    // optimized offset[n] = weight[n] + weight[n+1] // ex:  aBloomoffset[1] =  0.1945945946 + 0.1216216216 = 0.3162162162 (except first and last)
    // Defualt values
    //aBloomweight[0] = 0.1945945946f;
    //aBloomweight[1] = 0.3162162162f;
    //aBloomweight[2] = 0.0702702703f;
    
    aBloomweight[0] = 0.3345945946f;
    aBloomweight[1] = 0.3362162162f;
    aBloomweight[2] = 0.1702702703f;
    
    // Offsets
    // Baseoffset represents offset for not optimized linear sampling
    aBaseoffset[0] = 0.0f;
    aBaseoffset[1] = 1.0f;
    aBaseoffset[2] = 2.0f;
    aBaseoffset[3] = 3.0f;
    aBaseoffset[4] = 4.0f;
    
    // Optimized offset are also recalculated later because it is possible to modify weights in params
    aBloomoffset[0] = 0.0f; // not for the first one
    // = 1.3846153846
    aBloomoffset[1] = ( aBaseoffset[1] * aBaseweight[1] + aBaseoffset[2] * aBaseweight[2]) / aBloomweight[1];
    // = 3.23076923044 rounded to 3.2307692308
    aBloomoffset[2] = ( aBaseoffset[3] * aBaseweight[3] + aBaseoffset[4] * aBaseweight[4] ) / aBloomweight[2];
    
    int maxPatchVertices = 0;
    glGetIntegerv( GL_MAX_PATCH_VERTICES, &maxPatchVertices );
    app::console() << "Max supported patch vertices " << maxPatchVertices << std::endl;
    
    mHeightTex = gl::TextureCubeMap::create( loadImage( loadAsset( glDir / "heightmap.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );
    mOccTex = gl::TextureCubeMap::create( loadImage( loadAsset( glDir / "occlusionmap.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );
    mTranslucencyTex = gl::TextureCubeMap::create( loadImage( loadAsset( glDir / "translucencymap.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );
    mReflexionTex = gl::TextureCubeMap::create( loadImage( loadAsset( glDir / "reflexionmap.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );

    mReflexionTex->bind( 0 );
    mHeightTex->bind( 1 );
    mOccTex->bind( 2 );
    mTranslucencyTex->bind( 3 );
    
    
    // Tesselation Batch
    auto format = gl::GlslProg::Format()
    .vertex( loadAsset( "shader.vert" ) )
    .fragment( loadAsset( "shader.frag" ) )
    .geometry( loadAsset( "shader.geom" ) )
    .tessellationCtrl( loadAsset( "shader.cont" ) )
    .tessellationEval( loadAsset( "shader.eval" ) );
    auto shader	= gl::GlslProg::create( format );
    mBatch		= gl::Batch::create( mCubeSphere.create(), shader );
    
    // Bloom Batch
    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );
    DataSourceRef fragBloom         = loadAsset( "bloom.frag" );
    DataSourceRef vertPassThrough	= loadAsset( "pass_through.vert" );
    gl::GlslProgRef bloom           = loadGlslProg( gl::GlslProg::Format().vertex( vertPassThrough ).fragment( fragBloom ));
    mBatchBloomRect                 = gl::Batch::create( rect, bloom );
    mBatchBloomRect->getGlslProg()->uniform( "uSampler", 0 );
    
    // Final render: scene and bloom textures with additive blending and tone mapping
    DataSourceRef fragComposite    = loadAsset( "composite.frag" );
    gl::GlslProgRef composite      = loadGlslProg( gl::GlslProg::Format().vertex( vertPassThrough ).fragment( fragComposite ));
    mBatchFinalRenderRect          = gl::Batch::create( rect, composite );
    mBatchFinalRenderRect->getGlslProg()->uniform(	"uSamplerScene", 0 );
    mBatchFinalRenderRect->getGlslProg()->uniform(	"uSamplerBloom", 1 );
    mBatchFinalRenderRect->getGlslProg()->uniform(	"uSamplerLight", 2 );
    
    // Fxaa Batch
    DataSourceRef fragFxaa	= loadAsset( "fxaa.frag" );
    gl::GlslProgRef postFxaa	= loadGlslProg( gl::GlslProg::Format().vertex( vertPassThrough ).fragment( fragFxaa ) );
    mBatchFxaaRect              = gl::Batch::create( rect, postFxaa );
    mBatchFxaaRect->getGlslProg()->uniform(	"uSampler",	0 );
    
    // To screen Batch
    gl::GlslProgRef stockTexture	= gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchToScreenRect			= gl::Batch::create( rect, stockTexture );
    
    // Tessalation
    mInnerLevel = 1.0f;
    mOuterLevel = 1.0f;
    
    // connect the keydown signal
    getWindow()->getSignalKeyDown().connect( [this](KeyEvent event) {
        switch ( event.getCode() ) {
            case KeyEvent::KEY_LEFT : mInnerLevel--; break;
            case KeyEvent::KEY_RIGHT : mInnerLevel++; break;
            case KeyEvent::KEY_DOWN : mOuterLevel--; break;
            case KeyEvent::KEY_UP : mOuterLevel++; break;
            case KeyEvent::KEY_s :
            mSaveScreen = !mSaveScreen;
            break;
            case KeyEvent::KEY_f :
            setFullScreen( !isFullScreen() );
            resize();
            break;
            case KeyEvent::KEY_m : mDisplacementCoef = mDisplacementCoef + 0.05f; break;
            case KeyEvent::KEY_n : mDisplacementCoef = mDisplacementCoef - 0.05f; break;
        }
        mInnerLevel = math<float>::max( mInnerLevel, 1.0f );
        mOuterLevel = math<float>::max( mOuterLevel, 1.0f );
    });
    
    // Params
    mParams = params::InterfaceGl::create( getWindow(), "Settings", toPixels( ivec2( 230, 220 ) ) );
    mParams->addParam( "Bloomweight 0", &aBloomweight[0], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 1", &aBloomweight[1], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Bloomweight 2", &aBloomweight[2], "min=-1.0 max=5.0 step=0.000001 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "HDR exposure", &mExposure, "min= -1.0 max=5.0 step=0.01 keyIncr=q keyDecr=s" );
    mParams->addParam( "HDR gamma", &mGamma, "min= 0.0 max=5.0 step=0.01 keyIncr=q keyDecr=s" );
    mParams->addSeparator();
    mParams->addParam( "HDR light", &mLight, "min= -5.0 max=20.0 step=0.01 keyIncr=q keyDecr=s" );
    resize();
    
    cinder::app::console() <<  "START >>";
}


void CubeSphereDisplacementMappingApp::resize()
{
    mCam.lookAt( vec3( 3.0f, 0.0f, 0.0f), vec3( 0 ) );
    mCam.setPerspective( 60, mFboSize.x/mFboSize.y, 0.1, 10 );
    
    // Choose window size based on selected quality
    ivec2 winSize	= toPixels( getWindowSize() );
    //int32_t h		= winSize.y;
    //int32_t w		= winSize.x;

    int32_t h		= mFboSize.y;
    int32_t w		= mFboSize.x;
    
    cinder::app::console() <<  "RESIZE >> Y:" << h << " Y:" << w;
    
    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );
    
    // Set up the buffer for rendering
    // 0 GL_COLOR_ATTACHMENT1	scene
    // 1 GL_COLOR_ATTACHMENT1	scene for bloom
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 5; ++i ) {
            mTexturemRender[ i ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                         .internalFormat( GL_RGBA16F_ARB )
                                                         .magFilter( GL_LINEAR )
                                                         .minFilter( GL_LINEAR )
                                                         .wrap( GL_CLAMP_TO_EDGE )
                                                         .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTexturemRender[ i ] );
        }
        
        mRenderFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        gl::clear();
        
    }
    
    // Ping Pong FBO for Bloom Pass
    {
        // must adjust size to half the screen
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
}


void CubeSphereDisplacementMappingApp::mouseDown( MouseEvent event )
{
    mChange = !mChange;
}


void CubeSphereDisplacementMappingApp::update()
{
    mCubeRotation *= rotate( 0.01f,  vec3( 0.0f, 1.5f, 0.5f )  );
    
    mLightPosWorldSpace = vec3( 0.0f, 0.0f , 0.0f );
    
    mLight = 2.0f * ( ( 1.0f + sin( 0.5f * mCounter ) ) * 0.5f );
    
    // Save render fbo
    if ( mSaveScreen == true ){
        saveImage();
    }
    
    // 60 fps
    mCounter += 0.01667f;
    
    // 30 fps
    //mCounter += 0.021f;
    
    mCurrentFrame++;
}


void CubeSphereDisplacementMappingApp::draw()
{
    gl::enableDepthWrite();
    gl::enableDepthRead();
    gl::disableBlending();
    
    // Render scene in 2 textures ( GL_COLOR_ATTACHMENT1 for the bloom pass )
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        
        // Draw buffers
        {
            const static GLenum buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
            };
            gl::drawBuffers( 3, buffers );
        }
        gl::clear( Color( 0, 0, 0 ) );
        
        // Draw sphere
        {
            gl::setMatrices( mCam );
            gl::ScopedModelMatrix modelScope;
            gl::multModelMatrix( mCubeRotation );
            
            // Update uniforms
            mBatch->getGlslProg()->uniform( "uTessLevelInner", mInnerLevel );
            mBatch->getGlslProg()->uniform( "uTessLevelOuter", mOuterLevel );
            mBatch->getGlslProg()->uniform( "uDisplacement", mDisplacementCoef );
            mBatch->getGlslProg()->uniform( "uReflexionMap", 0 );
            mBatch->getGlslProg()->uniform( "uHeightMap", 1 );
            mBatch->getGlslProg()->uniform( "uOccMap", 2 );
            mBatch->getGlslProg()->uniform( "uTranslucencyMap", 3 );
            
            mBatch->getGlslProg()->uniform( "uLightLocViewSpace", vec3( mCam.getViewMatrix() * vec4( mLightPosWorldSpace, 1 )) );
            
            // Bypass gl::Batch::draw method so we can use GL_PATCHES
            gl::ScopedVao scopedVao( mBatch->getVao().get() );
            gl::ScopedGlslProg scopedShader( mBatch->getGlslProg() );
            
            gl::context()->setDefaultShaderVars();
            
            if( mBatch->getVboMesh()->getNumIndices() )
            glDrawElements( GL_PATCHES, mBatch->getVboMesh()->getNumIndices(), mBatch->getVboMesh()->getIndexDataType(), (GLvoid*)( 0 ) );
            else
            glDrawArrays( GL_PATCHES, 0, mBatch->getVboMesh()->getNumIndices() );
        }
    }
    
    // Ping pong bloom vars
    bool horizontal = true;
    bool first_iteration = true;
    int numberOfPasses = 3;
    
    // Bloom
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mBloomFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mBloomFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mBloomFBO->getSize() );
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( (float)mBloomFBO->getWidth()/2.0f, (float)mBloomFBO->getHeight()/2.0f );
        gl::scale( mBloomFBO->getWidth(), mBloomFBO->getHeight() );
        
        // Run horizontal blur pass and vertical blur pass
        for ( int i = 0; i < numberOfPasses; i++ )
        {
            gl::drawBuffer( GL_COLOR_ATTACHMENT0  + (GLenum)horizontal );
            gl::clear( Color( 0, 0, 0 ) );
            int height = mBloomFBO->getHeight();
            int width = mBloomFBO->getWidth();
            
            if ( first_iteration )
            {
                mTexturemRender[ 1 ]->bind(0);
            }
            else mTextureBloom[(int)!horizontal]->bind(0);
            
            mBatchBloomRect->getGlslProg()->uniform( "weight", aBloomweight, 3 );
            mBatchBloomRect->getGlslProg()->uniform( "offset", aBloomoffset, 3 );
            mBatchBloomRect->getGlslProg()->uniform( "horizontal", horizontal );
            mBatchBloomRect->getGlslProg()->uniform( "width", width );
            mBatchBloomRect->getGlslProg()->uniform( "height", height );
            mBatchBloomRect->draw();
            
            if ( first_iteration ) mTexturemRender[1]->unbind(0);
            else mTextureBloom[(int)!horizontal]->unbind(0);
            
            horizontal = !horizontal;
            if ( first_iteration )
            first_iteration = false;
        }
    }
    
    // Final render: additive blending + tone mapping
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mRenderFBO->getSize() );
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::translate( mRenderFBO->getSize() / 2 );
        gl::scale( mRenderFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT3 );
        gl::clear( Color( 0, 0, 0 ) );
        
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 0 ], 0 ); // scene color texture
        const gl::ScopedTextureBind scopedTextureBind1( mTextureBloom[(int)!horizontal], 1 ); // bloom texture
        const gl::ScopedTextureBind scopedTextureBind2( mTexturemRender[ 2 ], 2 ); // light texture
        mBatchFinalRenderRect->getGlslProg()->uniform( "exposure", mExposure );
        mBatchFinalRenderRect->getGlslProg()->uniform( "gamma", mGamma );
        mBatchFinalRenderRect->getGlslProg()->uniform( "light", mLight );
        mBatchFinalRenderRect->draw();
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
    
    // Render our final image to the screen
    {
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), toPixels( getWindowSize() ) );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( toPixels( getWindowSize() ) );
        gl::translate( toPixels( getWindowSize() / 2 )  );
        gl::scale( toPixels( getWindowSize() ) );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        //const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 2 ], 0 );
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 4 ], 0 );
        
        //const gl::ScopedTextureBind scopedTextureBind1( mTextureBloom[(int)!horizontal], 0 );
        mBatchToScreenRect->draw();
    }
    
    mParams->draw();
    getWindow()->setTitle( "Framerate: " + to_string( (int) getAverageFps() ) );
}


// Save renderFbo
void CubeSphereDisplacementMappingApp::saveImage()
{
    // Pull down the current window as a surface and pass it to writeImage
    writeImage( getHomeDirectory() / "CubeSphere" / ( toString(1) + "_" + toString( mCurrentFrame ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
}

CINDER_APP( CubeSphereDisplacementMappingApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
