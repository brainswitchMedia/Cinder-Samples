#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class self_illuminated_TranslucencyApp : public App {
  public:
    static void prepareSettings( Settings *settings );
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    void resize() override;

    gl::VboMeshRef  createPrimitiveFromCube( bool dual );
    void createGeometry();
    void loadShaders();
    void saveImage();

    vec3 mUnitCube[8];
    // Tetraedre buffers
    cinder::gl::VboMeshRef      mVboMesh;
    std::vector<uint16_t>       mIndices;
    std::vector<ci::vec3>       mPosCoords;
    std::vector<ci::vec3>       mNormals;
    
    CameraPersp mCamera;
    vec3 mCameraTarget = vec3( 0.0, 0.0, 0.0 );
    mat4 mSceneRotation;

    float mCounter_1 = 0.0f;
    
    vec2 mFboSize = vec2 (1080, 1080);
    //vec2 mFboSize = vec2 (1280, 720);
    //vec2 mFboSize = vec2 (3840, 2160);
    
    gl::FboRef mRenderFBO;
    ci::gl::Texture2dRef mTexturemRender[ 2 ];
    gl::BatchRef mPrimitive[2], mBatchFxaaRect, mBatchToScreenRect;
    gl::GlslProgRef mIlluminationShader, mFxaaShader;
    
    int mCurrentFrame = 0;
    bool mSaveScreen = false;
    std::string mKeyPressed;
    
    params::InterfaceGlRef mParams;
    float mBrightness = 1.279f;
    float mSize = 1.850f;
    float mLightPower = 0.02f;
    float mDistancePower = 1.38f;

};


void self_illuminated_TranslucencyApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 30 );
    settings->setWindowSize( 1280, 720 );
}


void self_illuminated_TranslucencyApp::setup()
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
    
    loadShaders();

    createGeometry();
    
    // connect the keydown signal
    mKeyPressed = "nul";
    getWindow()->getSignalKeyDown().connect( [this]( KeyEvent event )
    {
        switch ( event.getCode() )
        {
            case KeyEvent::KEY_s :
                mSaveScreen = !mSaveScreen;
                break;
            case KeyEvent::KEY_f :
                setFullScreen( !isFullScreen() );
                resize();
                break;
        }
    });
    mCamera.setPerspective( 35.0f, getWindowAspectRatio(), 0.01f, 10.0f );
    mCamera.lookAt( vec3( 4, 4, 4 ), vec3( 0, 0, 0 ) );
    
    mParams = params::InterfaceGl::create( getWindow(), "Geometry Demo", toPixels( ivec2( 300, 400 ) ) );
    mParams->setOptions( "", "valueswidth=100 refresh=0.1" );
    mParams->addParam( "Size ", &mSize, "min=-100.0 max= 100.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Brightness ", &mBrightness, "min=-1000.0 max= 1000.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Light Power ", &mLightPower, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Distance Power ", &mDistancePower, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );

}


gl::VboMeshRef self_illuminated_TranslucencyApp::createPrimitiveFromCube( bool dual )
{
    mPosCoords.clear();
    mIndices.clear();
    mNormals.clear();
    
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
    
    // Vertex position from unit cube
    vector<vec3> posCoords;
    if ( dual == false )
    {
        posCoords.push_back( mUnitCube[5] );
        posCoords.push_back( mUnitCube[7] );
        posCoords.push_back( mUnitCube[2] );
        posCoords.push_back( mUnitCube[0] );
        
        // Create GL_TRIANGLES vertex with unit cube
        mPosCoords.push_back( posCoords[1] );
        mPosCoords.push_back( posCoords[2] );
        mPosCoords.push_back( posCoords[0] );
        
        mPosCoords.push_back( posCoords[2] );
        mPosCoords.push_back( posCoords[3] );
        mPosCoords.push_back( posCoords[0] );
        
        mPosCoords.push_back( posCoords[3] );
        mPosCoords.push_back( posCoords[1] );
        mPosCoords.push_back( posCoords[0] );
        
        mPosCoords.push_back( posCoords[1] );
        mPosCoords.push_back( posCoords[3] );
        mPosCoords.push_back( posCoords[2] );
    }
    else
    {
        posCoords.push_back( mUnitCube[1] );
        posCoords.push_back( mUnitCube[3] );
        posCoords.push_back( mUnitCube[6] );
        posCoords.push_back( mUnitCube[4] );
        
        // Create GL_TRIANGLES vertex with unit cube
        mPosCoords.push_back( posCoords[0] );
        mPosCoords.push_back( posCoords[2] );
        mPosCoords.push_back( posCoords[1] );
        
        mPosCoords.push_back( posCoords[0] );
        mPosCoords.push_back( posCoords[3] );
        mPosCoords.push_back( posCoords[2] );
        
        mPosCoords.push_back( posCoords[0] );
        mPosCoords.push_back( posCoords[1] );
        mPosCoords.push_back( posCoords[3] );
        
        mPosCoords.push_back( posCoords[2] );
        mPosCoords.push_back( posCoords[3] );
        mPosCoords.push_back( posCoords[1] );
    }
    

    
    // Create indices matching with vertex coords
    for( size_t i = 0; i < mPosCoords.size(); ++i ) {
        
        mIndices.push_back( i );
    }
    
    // Normals: calculate the face normal for each triangle
    size_t numTriangles = mIndices.size() / 3;
    console() << "numTriangles: " << numTriangles << std::endl;
    
    for( size_t i = 0; i < numTriangles; ++i ) {
        const uint32_t index0 = mIndices[i*3+0];
        const uint32_t index1 = mIndices[i*3+1];
        const uint32_t index2 = mIndices[i*3+2];
        
        const vec3 &v0 = mPosCoords[index0];
        const vec3 &v1 = mPosCoords[index1];
        const vec3 &v2 = mPosCoords[index2];
        
        vec3 e0 = v1 - v0;
        vec3 e1 = v2 - v0;
        
        mNormals.push_back( normalize( cross( e0, e1 )));
        mNormals.push_back( normalize( cross( e0, e1 )));
        mNormals.push_back( normalize( cross( e0, e1 )));
    }
    
    // Layout
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
    
    return mVboMesh;
}


void self_illuminated_TranslucencyApp::mouseDown( MouseEvent event )
{
}


void self_illuminated_TranslucencyApp::update()
{
    mSceneRotation *= rotate( 0.015f, normalize(vec3( 1.5f, 2.0f, 1.0f ))  );
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
    
    // 30 fps
    //mCounter_1 += 0.03333f;
    mCurrentFrame++;
    

}


void self_illuminated_TranslucencyApp::draw()
{
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
            };
            gl::drawBuffers( 1, buffers );
        }
        gl::clear();
        
        // Set Matrices
        gl::setMatrices( mCamera );
        gl::ScopedModelMatrix modelScope;
        
        // Render Frontside only
        //gl::ScopedFaceCulling cullScope( true, GL_BACK );
        //gl::cullFace( GL_BACK );
        
        // Render Tetraedres
        for( int i = 0; i < 2; ++i )
        {
            // Render the back side first.
            gl::ScopedBlendAlpha blendScope;
            gl::ScopedFaceCulling cullScope( true, GL_FRONT );
            
            gl::ScopedModelMatrix scpModelMatrix;
            gl::multModelMatrix( mSceneRotation );
            mPrimitive[i]->getGlslProg()->uniform( "uBrightness", mBrightness );
            mPrimitive[i]->getGlslProg()->uniform( "uSize", mSize );
            mPrimitive[i]->getGlslProg()->uniform( "uPower1", mLightPower );
            mPrimitive[i]->getGlslProg()->uniform( "uPower2", mDistancePower );

            mPrimitive[i]->getGlslProg()->uniform( "uLightModelView", mCamera.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
            mPrimitive[i]->draw();
        }
        for( int i = 0; i < 2; ++i )
        {
            gl::ScopedBlendAlpha blendScope;
            gl::cullFace( GL_BACK );            
            gl::ScopedModelMatrix scpModelMatrix;
            gl::multModelMatrix( mSceneRotation );
            mPrimitive[i]->getGlslProg()->uniform( "uBrightness", mBrightness );
            mPrimitive[i]->getGlslProg()->uniform( "uSize", mSize );
            mPrimitive[i]->getGlslProg()->uniform( "uPower1", mLightPower );
            mPrimitive[i]->getGlslProg()->uniform( "uPower2", mDistancePower );
            
            mPrimitive[i]->getGlslProg()->uniform( "uLightModelView", mCamera.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
            mPrimitive[i]->draw();
        }
    }
    
    // Fxaa
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mRenderFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mRenderFBO->getSize() );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mRenderFBO->getSize() );
        gl::translate( mRenderFBO->getSize() / 2 );
        gl::scale( mRenderFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
        gl::clear( Color( 0, 0, 0 ) );
        // To keep bandwidth in check, we aren't using any hardware
        // anti-aliasing (MSAA). Instead, we use FXAA as a post-process
        // to clean up our image.
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 0 ], 0 ); // final render texture
        mBatchFxaaRect->getGlslProg()->uniform( "uPixel", 1.0f / vec2( mRenderFBO->getSize() ) );
        mBatchFxaaRect->draw();
    }
    
    // Test Render to the screen
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
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 1 ], 0 );
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureGodRays[ 0 ], 0 );
        mBatchToScreenRect->draw();
    }

    // Render the parameter windows.
    if( mParams ) {
        mParams->draw();
    }
    
    // Save screen
    if ( mSaveScreen == true ){
        saveImage();
    }
}


void self_illuminated_TranslucencyApp::resize()
{
    mCamera.setAspectRatio( mFboSize.x / mFboSize.y );
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

}


void self_illuminated_TranslucencyApp::createGeometry()
{
    // Create center Tetraedre to draw
    mPrimitive[0] = gl::Batch::create( createPrimitiveFromCube( false ), mIlluminationShader );
    mPrimitive[1] = gl::Batch::create( createPrimitiveFromCube( true ), mIlluminationShader );

    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );

    // Fxaa Batch
    mBatchFxaaRect = gl::Batch::create( rect, mFxaaShader );
    mBatchFxaaRect->getGlslProg()->uniform(	"uSampler",	0 );
    
    // To screen Batch
    gl::GlslProgRef stockTexture    = gl::context()->getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ) );
    mBatchToScreenRect              = gl::Batch::create( rect, stockTexture );
}


void self_illuminated_TranslucencyApp::loadShaders()
{
    try {
        mIlluminationShader = gl::GlslProg::create( loadAsset( "illumination.vert" ), loadAsset( "illumination.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Illumination shader: " << exc.what() );
    }
    
    try {
        mFxaaShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "fxaa.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Fxaa shader: " << exc.what() );
    }
}


// Save renderFbo
void self_illuminated_TranslucencyApp::saveImage()
{
    // Pull down the current window as a surface and pass it to writeImage
    writeImage( getHomeDirectory() / "deformedSphere" / ( toString(1) + "_" + toString( mCurrentFrame ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT1 )->createSource(), ImageTarget::Options().quality(1.0f) );
}


CINDER_APP( self_illuminated_TranslucencyApp, RendererGl, &self_illuminated_TranslucencyApp::prepareSettings )
