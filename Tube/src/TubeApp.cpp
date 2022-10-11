#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "TubeManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TubeApp : public App {
  public:
    static void prepareSettings( Settings *settings );
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    void resize() override;

    vec3 computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 );
    void computeParticlesEmittersPosition();

    void createGeometry();
    void loadShaders();
    void loadTextures();
    void saveImage();
    
    vec3 rotateAxis( bool beat, vec3 &rotationVectorFrom, vec3 &rotationVectorTo, float easeSpeed );
    vec3 slerp( vec3 &start, vec3 &end, float percent );
    void shortenTubes( bool start );
    
    // Tubes
    TubeManager             mTubemanager;
    vec3                    mRotationVector = vec3( 0.0f, 1.0f, 0.0f );
    std::vector<vec3>       mParticlesEmitters;
    bool                    mStartShortenTubes;
    
    // Tetraedron buffers
    vec3 mUnitCube[8];
    cinder::gl::VboMeshRef  mVboMesh;
    std::vector<uint16_t>   mIndices;
    std::vector<ci::vec3>   mPosCoords;
    std::vector<ci::vec3>   mNormals;
    
    //vec2                    mFboSize = vec2 ( 1080, 1080 );
    vec2                    mFboSize = vec2 ( 540, 540 );

    // Scene Rotation
    bool                    mBeat;
    float                   mbeatCounter;
    mat4                    mSceneRotation;
    vec3                    rotationVector1;
    vec3                    rotationVector2;
    int                     mChangeRotation;

    //Render
    gl::FboRef              mRenderFBO, mDitherFBO, mPixelArtUpscaleFBO ;
    gl::BatchRef            mPrimitive, mBatchUpscaleRect, mBatchDitherRect, mBatchToScreenRect;
    gl::GlslProgRef         mDitheringShader, mPixelArtUpscalingShader;
    ci::gl::Texture2dRef    mTexturemRender[ 2 ];
    ci::gl::Texture2dRef    mTextureDither[ 1 ];
    ci::gl::Texture2dRef    mTexturePixelUpScale[ 1 ];
    ci::gl::TextureRef      mTextureBlurNoise;
    
    // Other
    CameraPersp             mCam;
    int                     mCurrentFrame = 0;
    int                     mSavedFrame = 0;
    bool                    mSaveScreen = false;
    float                   mCounter_1;
    float                   mCounterCycle;
    std::string             mKeyPressed;
    bool                    m30fps = true;

    // Params
    params::InterfaceGlRef  mParams;
    float                   mBrightness = 1.279f;
    float                   mSize = 1.850f;
    float                   mLightPower = 0.02f;
};


void TubeApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 60 );
    settings->setWindowSize( 1080, 1080 );
}


void TubeApp::setup()
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
    
    computeParticlesEmittersPosition();
    
    // Load textures
    loadTextures();
    
    // Load and compile the shaders
    loadShaders();
    
    // Create the meshes.
    createGeometry();
    
    // connect the keydown signal
    mKeyPressed = "nul";
    getWindow()->getSignalKeyDown().connect( [this]( KeyEvent event )
                                            {
                                                switch ( event.getCode() )
                                                {
                                                    case KeyEvent::KEY_a :
                                                        mTubemanager.mTubesParams.mAutoGrowLength = !mTubemanager.mTubesParams.mAutoGrowLength;
                                                        break;
                                                    case KeyEvent::KEY_b :
                                                        mStartShortenTubes = true;
                                                        mTubemanager.mTubesParams.mAutoGrowLength = false;
                                                        break;
                                                    case KeyEvent::KEY_s :
                                                        mSaveScreen = !mSaveScreen;
                                                        break;
                                                    case KeyEvent::KEY_f :
                                                        setFullScreen( !isFullScreen() );
                                                        resize();
                                                        break;
                                                }
                                            });
    
    // Camera && scene
    mCam.lookAt( vec3( 2.0, 0, 0 ), vec3( 0 ) );
    mCounter_1 = 0.0f;
    mCounterCycle = 0.0f;
    mChangeRotation = 0;
    rotationVector1 = normalize ( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
    vec3 rotationVectorTmp = normalize ( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
    rotationVector2 = cross( rotationVector1, rotationVectorTmp );

    // Tubes
    mTubemanager.setup( "tubeColor.vert", "tubeColor.frag", "textures/gradiant.png", "textures/tube_stripes2.jpg");
    mStartShortenTubes = false;
    
    // Scene rotation
    mBeat = false;
    
    // Params
    mParams = params::InterfaceGl::create( getWindow(), "Geometry Demo", toPixels( ivec2( 300, 400 ) ) );
    mParams->setOptions( "", "valueswidth=100 refresh=0.1" );
    mParams->addParam( "Size ", &mSize, "min=-100.0 max= 100.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Brightness ", &mBrightness, "min=-1000.0 max= 1000.0 step=0.001 keyIncr=q keyDecr=w" );
}


void TubeApp::mouseDown( MouseEvent event )
{
    mBeat = true;
}


void TubeApp::update()
{
    // Scene rotation
    vec3 rotationAxis = rotateAxis( mBeat, rotationVector1, rotationVector2, 0.010f );
    mSceneRotation *= rotate( 0.05f, rotationAxis );

    int tubeNumber = mParticlesEmitters.size();
    std::vector<vec3> particleEmitter;
    
    for ( int i = 0; i < tubeNumber; i++ )
    {
        particleEmitter.push_back( mat3( mSceneRotation ) * mParticlesEmitters[i] * 0.001f );
    }
    
    // One Tube for each particle emiter
    mTubemanager.update( particleEmitter, mRotationVector );
    mBeat = false;
    
    shortenTubes( mStartShortenTubes );
    
    // Render the parameter windows.
    if( mParams ) {
        mParams->draw();
    }
    
    // Save screen
    if ( mSaveScreen == true ){
        saveImage();
    }
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
    mCounterCycle += 0.01667f;
    if ( mCounterCycle > 0.5f )
    {
        mCounterCycle = 0.0f;
       // mBeat = true;
    }
    
    mCurrentFrame++;
}


void TubeApp::draw()
{
    // Render scene
    {
        gl::enableDepthRead(true);
        gl::enableDepthWrite(true);
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        // Set Draw buffers
        {
            const static GLenum buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
            };
            gl::drawBuffers( 3, buffers );
        }
        
        gl::clear();
        gl::clear( Color( 0.01, 0.01, 0.01 ) );
        gl::setMatrices( mCam );
        gl::ScopedModelMatrix modelScope;
        mTubemanager.drawTubes( mCounter_1 );
    }
    
    // Dithering
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mDitherFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mDitherFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mDitherFBO->getSize() );
        gl::translate( mDitherFBO->getSize() / 2 );
        gl::scale( mDitherFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        gl::clear( Color( 0, 0, 0 ) );
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 0 ], 0 );
        const gl::ScopedTextureBind scopedTextureBind1( mTextureBlurNoise, 1 ); // Blue noise texture
        mBatchDitherRect->getGlslProg()->uniform( "uTime", 0.0f );
        mBatchDitherRect->draw();
    }
    
    // Pixel Art Upscaling
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mPixelArtUpscaleFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mPixelArtUpscaleFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mPixelArtUpscaleFBO->getSize() );
        gl::translate( mPixelArtUpscaleFBO->getSize() / 2 );
        gl::scale( mPixelArtUpscaleFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        gl::clear( Color( 0, 0, 0 ) );
        const gl::ScopedTextureBind scopedTextureBind0( mTextureDither[ 0 ], 0 ); // dithered texture
        mBatchUpscaleRect->getGlslProg()->uniform( "uTextureSize", vec2( mRenderFBO->getSize() ) );
        mBatchUpscaleRect->draw();
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

        // Uncomment to see normals
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 1 ], 0 );
        
        // Uncommnt to see dithering effect
        //const gl::ScopedTextureBind scopedTextureBind0( mTexturePixelUpScale[ 0 ], 0 );
        mBatchToScreenRect->draw();
    }
}


void TubeApp::resize()
{
    mCam.setAspectRatio( mFboSize.x / mFboSize.y );
    // Choose window size based on selected quality
    //ivec2 winSize       = toPixels( getWindowSize() );
    //int32_t h         = winSize.y;
    //int32_t w         = winSize.x;
    int32_t h           = mFboSize.y;
    int32_t w           = mFboSize.x;
    
    cinder::app::console() <<  "RESIZE >> Y:" << h << " Y:" << w;
    
    gl::VboMeshRef  rect = gl::VboMesh::create( geom::Rect() );
    
    // Set up the buffer for rendering
    // 0 GL_COLOR_ATTACHMENT1	scene
    // 1 GL_COLOR_ATTACHMENT1	fxaa
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 2; ++i ) {
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
    
    
    // Dithering FBO
    {
        ivec2 sz = ivec2( w, h );
        
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 1; ++i ) {
            mTextureDither[ i ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                        .internalFormat( GL_RGBA16F_ARB )
                                                        .magFilter( GL_LINEAR )
                                                        .minFilter( GL_LINEAR )
                                                        .wrap( GL_CLAMP_TO_EDGE )
                                                        .dataType( GL_FLOAT )
                                                        .mipmap());
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTextureDither[ i ] );
        }
        
        mDitherFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mDitherFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mDitherFBO->getSize() );
        gl::clear();
    }
    
    
    // PIXEL ART UPSCALING FBO
    {
        ivec2 sz = 2 * ivec2( w, h );
        gl::Fbo::Format fboFormat;
        for ( size_t i = 0; i < 1; ++i ) {
            mTexturePixelUpScale[ i ] = gl::Texture2d::create( sz.x, sz.y,  gl::Texture2d::Format()
                                                              .internalFormat( GL_RGBA16F_ARB )
                                                              .magFilter( GL_NEAREST )
                                                              .minFilter( GL_NEAREST )
                                                              .wrap( GL_CLAMP_TO_EDGE )
                                                              .dataType( GL_FLOAT ));
            fboFormat.attachment( GL_COLOR_ATTACHMENT0 + (GLenum)i, mTexturePixelUpScale[ i ] );
        }
        mPixelArtUpscaleFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mPixelArtUpscaleFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mPixelArtUpscaleFBO->getSize() );
        gl::clear();
    }
    
}


void TubeApp::computeParticlesEmittersPosition()
{
    // Vertex
    //
    //     V2-----*         y
    //      :     |         |__x
    //   *-----V0 |       z/
    //      *.....V1
    //
    //  V3-----*
    //
    // Dual
    //      *-----V1         y
    //      :     |         |__x
    //   V2-----* |       z/
    //     v3.....*
    //
    //   *-----V0
    //
    //  mUnitCube[0] = vec3( -1.0, -1.0, 1.0 ); //1 - Vtet3
    //  mUnitCube[1] = vec3( -1.0, 1.0, 1.0 ); //2 - Vtet2 Dual
    //  mUnitCube[2] = vec3( -1.0, 1.0, -1.0 ); //3 - Vtet2
    //  mUnitCube[3] = vec3( -1.0, -1.0, -1.0 ); //4 - Vtet3 Dual
    //  mUnitCube[4] = vec3( 1.0, -1.0, 1.0 ); // 5 - Vtet0 Dual
    //  mUnitCube[5] = vec3( 1.0, 1.0, 1.0 ); // 6 - Vtet0
    //  mUnitCube[6] = vec3( 1.0, 1.0, -1.0 ); // 7 - Vtet1 Dual
    //  mUnitCube[7] = vec3( 1.0, -1.0, -1.0 ); // 8 - Vtet1
    
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[5], mUnitCube[2], mUnitCube[0]) );
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[5], mUnitCube[0], mUnitCube[7]) );
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[5], mUnitCube[7], mUnitCube[2]) );
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[7], mUnitCube[2], mUnitCube[0]) );

    // Dual
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[4], mUnitCube[6], mUnitCube[1]) ); // V0-1-2
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[4], mUnitCube[3], mUnitCube[1]) ); // V0-3-2
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[4], mUnitCube[6], mUnitCube[3]) ); // V0-1-3
    mParticlesEmitters.push_back( computeTriangleGravitycenter( mUnitCube[6], mUnitCube[1], mUnitCube[3]) ); // V1-2-3
}



vec3 TubeApp::computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 )
{
    return vec3( point1.x + point2.x + point3.x, point1.y + point2.y + point3.y, point1.z + point2.z + point3.z ) / 3.0f;
}


void TubeApp::createGeometry()
{
    gl::VboMeshRef  rect = gl::VboMesh::create( geom::Rect() );
    
    // Dithering Batch
    mBatchDitherRect = gl::Batch::create( rect, mDitheringShader );
    mBatchDitherRect->getGlslProg()->uniform( "uSampler", 0 );
    mBatchDitherRect->getGlslProg()->uniform( "uTexBlueNoise", 1 );
    
    // Pixel art upscale Batch
    mBatchUpscaleRect = gl::Batch::create( rect, mPixelArtUpscalingShader );
    mBatchUpscaleRect->getGlslProg()->uniform( "uSampler", 0 );
    
    // To screen Batch
    gl::GlslProgRef stockTexture = gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchToScreenRect = gl::Batch::create( rect, stockTexture );
}


void TubeApp::loadShaders()
{
    try {
        mDitheringShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "dither.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Dithering shader: " << exc.what() );
    }
    
    try {
        mPixelArtUpscalingShader = gl::GlslProg::create( loadAsset( "pixel_upscaling.vert" ), loadAsset( "pixel_upscaling.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Pixel Upscaling shader: " << exc.what() );
    }
}


void TubeApp::loadTextures()
{
    // Load the textures.
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    fmt.setWrap( GL_REPEAT, GL_REPEAT );
    mTextureBlurNoise = gl::Texture::create( loadImage( loadAsset( "textures/mask_540_540.png" ) ), fmt );
}


// Save renderFbo
void TubeApp::saveImage()
{
    // Pull down the current window as a surface and pass it to writeImage
    if( m30fps == true && mCurrentFrame % 2 == 0 )
    {
          //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mSavedFrame ) + ".png" ), mPixelArtUpscaleFBO->getTexture2d( GL_COLOR_ATTACHMENT0 )->createSource(), ImageTarget::Options().quality(1.0f) );
        mSavedFrame++;
    }
}


vec3 TubeApp::rotateAxis( bool beat, vec3 &rotationVectorFrom, vec3 &rotationVectorTo, float easeSpeed )
{
    if ( beat == true )
    {
        mbeatCounter = 0.0f;
        rotationVectorFrom = rotationVectorTo;
        vec3 rotationVectorTmp = normalize( vec3( Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ), Rand::randFloat( -1.0f, 1.0f ) ) );
        rotationVectorTo = normalize( cross( rotationVectorFrom, rotationVectorTmp ) );
       //console() << "rotationVectorTo:" << rotationVectorTo << std::endl;
    }
    
    float ease = easeOutExpo( clamp( mbeatCounter, 0.0f, 1.0f ) );
    if ( mbeatCounter < 1.0f ) mbeatCounter += easeSpeed;

    return slerp( rotationVectorFrom, rotationVectorTo, ease );
}


// Vector slerp
vec3 TubeApp::slerp( vec3 &start, vec3 &end, float percent )
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


void TubeApp::shortenTubes( bool start )
{
    if ( start == true && mTubemanager.mTubesParams.mTubeLength > 14 )
    {
        mTubemanager.mTubesParams.mTubeLength -= 1;
    }
}


CINDER_APP( TubeApp, RendererGl, &TubeApp::prepareSettings )
