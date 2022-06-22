#include "cinder/Camera.h"
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

#include "SphereParticle.h"
#include "InstanciedParticles.h"
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


class hoofeli03App : public App {
public:
    hoofeli03App();
    void prepareSettings( App::Settings* settings );
    gl::VboMeshRef  createPrimitiveFromCube();
    gl::VboMeshRef  createPrimitiveFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4 );
    gl::VboMeshRef  createWirePrimitiveFromCube();
    
    void            setSpheresPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices );
    void            computeSpheresTetraedreFace();
    void            computeSpheresLines( const vec3 &topCorner, const uint16_t oppositeFace, const vec3 &baseTrianlePoint1, const vec3 &baseTrianlePoint2, const vec3 &baseTrianlePoint3 );
    void            sortIndicesCopy( const std::vector<uint16_t> &indices );
    void            uniqueVertexsCopy( const std::vector<vec3> &vertexPositions );
    
    vec3            computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 );
    mat4            rotationAlign( vec3 d, vec3 z );
    float           computePointSegDist( const vec3 &x0, const vec3 &x1, const vec3 &x2 );
    vec3            projectPointToSegment( const vec3 &x0, const vec3 &x1, const vec3 &x2 );
    vec3            computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 );

    void setup() override;
    void resize() override;
    void update() override;
    void draw() override;
    void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    
private:
    void transition_0( bool start );
    void transition_1( bool start );
    void transition_2( bool start );
    void transition_3( bool start );
    void setXMLAnimationPhases();
    void loadShaders();
    void loadTextures();
    void saveImage();
    void createGeometry();
    void subdivide( int mSubdivision );
    void createParams();
    void createGridTest();
    
    // Tetraedre buffers
    cinder::gl::VboMeshRef      mVboMesh;
    std::vector<uint16_t>       mIndices;
    std::vector<ci::vec3>       mPosCoords;
    std::vector<ci::vec3>       mNormals;
    std::vector<ci::vec3>       mTangents;
    std::vector<ci::vec2>       mTexCoords;
    
    // Tetraedre vars
    vec3                        mUnitCube[8];
    int                         mSubdivisions = 5;
    
    // Spheres test
    //static const int            numSpheres = 32; // sub 1
    //static const int            numSpheres = 128; // sub 2
    //static const int            numSpheres = 512; // sub 3
    //static const int            numSpheres = 2048; // sub 4
    static const int            numSpheres = 8192; // sub 5
    //static const int            numSpheres = 32768; // sub 6
    
    std::vector<uint16_t>       mCheckedIndices;
    std::vector<ci::vec3>       mUniquePosCoords;
    gl::BatchRef                mSpheres[ numSpheres ];
    std::vector<SphereParticle> mSpheresPositions;
    std::vector<uint16_t>       mSpheresTetraedreFace;
    
    //Tetraedre vars
    float                       mFreq;
    float                       mFreq2;
    float                       mWaveCoef;
    
    // Instacied particles
    InstanciedParticles         mInstanciedParticles;
    
    // Camera
    vec3                        mCameraTarget = vec3( 0.0, 0.0, 0.0 );
    ReactiveCam                 mReactiveCam;
    bool                        mIsBeat;

    // Scene Rotation
    mat4                        mSceneRotation;
    
    // GLSL & Rendering
    gl::BatchRef                mPrimitive, mPrimitiveSubdivided, mBatchGodRays, mBatchBloomRect, mBatchToScreenRect, mBatchHdrRect, mBatchFxaaRect;
    gl::BatchRef                mSurroundingTetraedres[4];

    gl::GlslProgRef             mWireframeShader, mPhongShader, mGodRaysShader, mBloomShader, mHdrShader, mFxaaShader;
    bool                        mEnableFaceFulling;
    
    // FBO
    gl::FboRef                  mRenderFBO, mGodRaysFBO, mBloomFBO, mHdrFBO;
    ci::gl::Texture2dRef        mTexturemRender[ 6 ];
    ci::gl::Texture2dRef        mTextureGodRays[ 1 ];
    ci::gl::Texture2dRef        mTextureHdr[ 1 ];
    ci::gl::Texture2dRef        mTextureBloom[ 2 ];
    ci::gl::TextureRef          mTextureStripes, mTextureSpectrum, mTextureAlbedo, mTextureNormal, mTextureRoughness, mTextureLine, mTextureLigth;
    
    // FBO Size
    vec2                        mFboSize = vec2 (1024, 640);
    //vec2                        mFboSize = vec2 (2160, 2160);
    //vec2                        mFboSize = vec2 (3840, 2160);

    // Distance shader
    LightData                   mLightCamera;
    float                       mDistanceConverstion;
    float                       mRedCoef;
    float                       mSizeCoef;
    float                       mMode;
    float                       mColorIndex;
    
    // PBR
    float                       mMetallic;
    float                       mRoughness;
    
    // GodRays
    float                       mExposure;
    float                       mDecay;
    float                       mDensity;
    float                       mWeight;
    
    // Bloom
    float                       aBloomValues[ 5 ];
    
    // HDR
    float                       mHdrExposure;
    float                       mHdrGamma;
    float                       mMixBloomGodrays;
    float                       mChromaticAberration;
    
    // Params
    params::InterfaceGlRef      mParams;
    std::string                 mKeyPressed;
    
    // Animation
    // 0 InitPosition, 1 leave, 2 go back to InitPosition
    uint16_t                    mSurroundingTetraedreMove;
    float                       mSurroundingTetraedreMoveDist;
    float                       mCounter_1;
    bool                        mStart_Transition0, mStart_Transition1, mStart_Transition2, mStart_Transition3;
    
    // SOUND XML PARSER //////////////////////////////////////////
    XMLcom                      mXMLcom;
    bool                        mPlayXML;
    float                       mSoundEnergyHeightFactor, mSoundEnergy2;
    float                       mIncrease = 1.0f;
    float                       mbeatCounter_1 = 0.0f;
    float                       mbeatCounter_2 = 0.0f;
    float                       mbeatCounter_3 = 1.0f;
    float                       mWaveCounter = 0.122f;

    float                       mEnergyCoef_1 = 0.2f;
    float                       mHdrGammaCounter = 0.0f;

    // SAVE IMAGE ///////////////////////////////////////////////
    int                         mCurrentFrame = 0;
    bool                        mSaveScreen = false;
    
    // Test
    gl::VertBatchRef mGrid;
    vec3 mGridP0;
    vec3 mGridP1;
};


void prepareSettings( App::Settings* settings )
{
    settings->setWindowSize( 1024, 768 );
    settings->setHighDensityDisplayEnabled();
    settings->setMultiTouchEnabled( false );
    settings->disableFrameRate();
}


hoofeli03App::hoofeli03App()
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


gl::VboMeshRef hoofeli03App::createPrimitiveFromCube()
{
    mPosCoords.clear();
    mIndices.clear();
    mTexCoords.clear();
    mNormals.clear();
    mTangents.clear();
    
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
    
    // mUniteCube Index
    //      2-----6         y
    //      |     |         |__x
    //   1-----5  |       z/
    //      3.....7
    //
    //   0-----4
    
    // Vertex position from unit cube
    vector<vec3> posCoords;
    posCoords.push_back( mUnitCube[5] );
    posCoords.push_back( mUnitCube[7] );
    posCoords.push_back( mUnitCube[2] );
    posCoords.push_back( mUnitCube[0] );
    
    // Create GL_TRIANGLES vertex with unit cube
    // Face 1 ( mUnitCube[7], mUnitCube[2], mUnitCube[5] ) -> opoosite point = posCoords[3] = mUnitCube[0]
    mPosCoords.push_back( posCoords[1] );
    mPosCoords.push_back( posCoords[2] );
    mPosCoords.push_back( posCoords[0] );
    // Face 2 ( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) -> opoosite point = posCoords[1] = mUnitCube[7]
    mPosCoords.push_back( posCoords[2] );
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[0] );
    // Face 3 ( mUnitCube[0], mUnitCube[7], mUnitCube[5] ) -> opoosite point = posCoords[2] = mUnitCube[2]
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[1] );
    mPosCoords.push_back( posCoords[0] );
    // Face 4 ( mUnitCube[7], mUnitCube[0], mUnitCube[2] ) -> opoosite point = posCoords[0] = mUnitCube[5]
    mPosCoords.push_back( posCoords[1] );
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[2] );
    
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
        
        console() << " normal p from c" << normalize( cross( e0, e1 )) << std::endl;

        
        mNormals.push_back( normalize( cross( e0, e1 )));
        mNormals.push_back( normalize( cross( e0, e1 )));
        mNormals.push_back( normalize( cross( e0, e1 )));
    }

    // UV Coords
    //
    //         V0c
    // V0c*     *     * V0b
    //
    //    V3 *     * V1
    //
    //          * V0c
    
    vec2 uv3a = vec2( 0, 0 );
    vec2 uv1  = vec2( 0.5f, 0);
    vec2 uv0  = vec2( 0.25f, sqrt( 0.75f )/2 );
    vec2 uv2  = vec2( 0.75f, sqrt( 0.75f )/2 );
    vec2 uv3b = vec2( 0.5f, sqrt( 0.75f ));
    vec2 uv3c = vec2( 1, 0 );
    
    // TextCoord
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv2 );
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv2 );
    mTexCoords.push_back( uv3b );
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv3a );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv2 );
    mTexCoords.push_back( uv3c );
    
    // Tangents
    //calculateTangents( mIndices.size(), mIndices.data(), mPosCoords.size(), mPosCoords.data(), mNormals.data(), mTexCoords.data(), &mTangents );
    
    // Layout
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    //layout.attrib( geom::TANGENT, 3 );
    layout.attrib( geom::TEX_COORD_0, 2 );//.usage( GL_STATIC_DRAW );
    
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    //mVboMesh->bufferAttrib( geom::TANGENT, mTangents.size() * sizeof( vec3 ), mTangents.data() );
    mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
    
    return mVboMesh;
}


gl::VboMeshRef hoofeli03App::createPrimitiveFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4 )
{
    mPosCoords.clear();
    mIndices.clear();
    mTexCoords.clear();
    mNormals.clear();
    mTangents.clear();
    

    
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
    
    // 5 7 2 0
    //mSurroundingTetraedres[0] = gl::Batch::create( createPrimitiveFromPoints( mUnitCube[6], mUnitCube[2], mUnitCube[5], mUnitCube[7] ), mPhongShader );

    
    // Vertex position from unit cube
    vector<vec3> posCoords;
    posCoords.push_back( point1 );
    posCoords.push_back( point2 );
    posCoords.push_back( point3 );
    posCoords.push_back( point4 );
    
    // Create GL_TRIANGLES vertex
    mPosCoords.push_back( posCoords[2] );
    mPosCoords.push_back( posCoords[0] );
    mPosCoords.push_back( posCoords[1] );
    
    mPosCoords.push_back( posCoords[2] );
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[0] );
    
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[1] );
    mPosCoords.push_back( posCoords[0] );
    
    /*mPosCoords.push_back( posCoords[1] );
    mPosCoords.push_back( posCoords[3] );
    mPosCoords.push_back( posCoords[2] );*/

    // Create indices matching with vertex coords
    for( size_t i = 0; i < (mPosCoords.size()); ++i ) {
        
        mIndices.push_back( i );
    }
    
    // Normals: calculate the face normal for each triangle
    size_t numTriangles = mIndices.size() / 3;
    
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
        
        console() << " normal p from p" << normalize( cross( e0, e1 )) << std::endl;

    }

    // UV Coords
    //
    //         V0c
    // V0c*     *     * V0b
    //
    //    V3 *     * V1
    //
    //          * V0c
    
    vec2 uv0  = vec2( 0.0f, 0.0f);
    vec2 uv1  = vec2( 1.0f, 0.0f);
    vec2 uv2  = vec2( 0.5f, 1.0f);

    
    // TextCoord
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv2 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv2 );
    mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv2 );
   /* mTexCoords.push_back( uv0 );
    mTexCoords.push_back( uv1 );
    mTexCoords.push_back( uv2 );*/

    // Layout
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    //layout.attrib( geom::TANGENT, 3 );
    layout.attrib( geom::TEX_COORD_0, 2 );//.usage( GL_STATIC_DRAW );
    
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
    
    return mVboMesh;
}


gl::VboMeshRef hoofeli03App::createWirePrimitiveFromCube()
{
    mPosCoords.clear();
    mIndices.clear();
    mTexCoords.clear();
    mNormals.clear();
    mTangents.clear();
    
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
    
    // mUniteCube Index
    //      2-----6         y
    //      |     |         |__x
    //   1-----5  |       z/
    //      3.....7
    //
    //   0-----4
    
    // If subdivide
    mPosCoords.push_back( mUnitCube[5] );
    mPosCoords.push_back( mUnitCube[7] );
    mPosCoords.push_back( mUnitCube[2] );
    mPosCoords.push_back( mUnitCube[0] );
    // Face 1 ( mUnitCube[7], mUnitCube[2], mUnitCube[5] )
    // -> opoosite point = posCoords[3] = mUnitCube[0]
    // -> cube Top Corner =  mUnitCube[6]
    mIndices.push_back( 1 );
    mIndices.push_back( 2 );
    mIndices.push_back( 0 );
    // Face 2 ( mUnitCube[2], mUnitCube[0], mUnitCube[5] )
    // -> opoosite point = posCoords[1] = mUnitCube[7]
    // -> cube Top Corner =  mUnitCube[1]
    mIndices.push_back( 2 );
    mIndices.push_back( 3 );
    mIndices.push_back( 0 );
    // Face 3 ( mUnitCube[0], mUnitCube[7], mUnitCube[5] )
    // -> opoosite point = posCoords[2] = mUnitCube[2]
    // -> cube Top Corner =  mUnitCube[4]
    mIndices.push_back( 3 );
    mIndices.push_back( 1 );
    mIndices.push_back( 0 );
    // Face 4 ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
    // -> opoosite point = posCoords[0] = mUnitCube[5]
    // -> cube Top Corner =  mUnitCube[3]
    mIndices.push_back( 1 );
    mIndices.push_back( 3 );
    mIndices.push_back( 2 );
    
    // Normals: calculate the face normal for each triangle
    size_t numTriangles = mIndices.size() / 3;
    console() << "numTriangles: " << numTriangles << std::endl;
    
    // subdivide all triangles
    subdivide( mSubdivisions );
    
    uniqueVertexsCopy( mPosCoords );
    setSpheresPositions( mPosCoords, mIndices );
    computeSpheresTetraedreFace();
    
    // Chose arbitrary a corner and his opposite base triangle from the tetraedre
    // Ex: Top Corner = mUnitCube[5] -> opposite triangle = Face 4 ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
    computeSpheresLines( mUnitCube[5], 1, mUnitCube[7], mUnitCube[0], mUnitCube[2] );
    
    // Layout
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
    
    return mVboMesh;
}


// Create positions for spheres on vertex positions and triangles gravity centers with vertex positions from a tetraedron
void hoofeli03App::setSpheresPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices )
{
    
    console() << " num mSpheres Positions .............. " << mSpheresPositions.size() << std::endl;
    
    size_t numUniquePosCoord = mUniquePosCoords.size();
    
    for( uint16_t i = 0; i < numUniquePosCoord; ++i )
    {
        mSpheresPositions.push_back( SphereParticle( mUniquePosCoords[i] ) );
    }
    
    console() << " num mSpheres Positions " << mSpheresPositions.size() << std::endl;

    size_t numTriangles = indices.size() / 3;
    
    for( uint16_t i = 0; i < numTriangles; ++i ) {
        uint16_t index0 = indices[i * 3 + 0];
        uint16_t index1 = indices[i * 3 + 1];
        uint16_t index2 = indices[i * 3 + 2];
        
        vec3 triangleGravityCenter = computeTriangleGravitycenter( posCoords[index0], posCoords[index1], posCoords[index2] );
        mSpheresPositions.push_back( SphereParticle( triangleGravityCenter ) );
    }
}


// Find on which face of the tetraedon is each small sphere
void hoofeli03App::computeSpheresTetraedreFace()
{
    
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
    
    // tetraedre Corner = 0, 2, 5, 7
    // top Cube Corner = 1, 3, 4, 6
    
    // Face ( mUnitCube[7], mUnitCube[2], mUnitCube[5] )
    // -> opoosite point = posCoords[3] = mUnitCube[0]
    // -> cube Top Corner =  mUnitCube[6]
    
    // Face ( mUnitCube[2], mUnitCube[0], mUnitCube[5] )
    // -> opoosite point = posCoords[1] = mUnitCube[7]
    // -> cube Top Corner =  mUnitCube[1]
    
    // Face ( mUnitCube[0], mUnitCube[7], mUnitCube[5] )
    // -> opoosite point = posCoords[2] = mUnitCube[2]
    // -> cube Top Corner =  mUnitCube[4]
    
    // Face ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
    // -> opoosite point = posCoords[0] = mUnitCube[5]
    // -> cube Top Corner =  mUnitCube[3]
    
    
    vec3 cubeCorner0 = mUnitCube[1]; // -> face 0 = ( mUnitCube[2], mUnitCube[0], mUnitCube[5] )
    vec3 cubeCorner1 = mUnitCube[3]; // -> face 1 = ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
    vec3 cubeCorner2 = mUnitCube[4]; // -> face 2 = ( mUnitCube[0], mUnitCube[7], mUnitCube[5] )
    vec3 cubeCorner3 = mUnitCube[6]; // -> face 3 = ( mUnitCube[7], mUnitCube[2], mUnitCube[5] )
    
    int numSpheres = mSpheresPositions.size();
    
    float maxDistToFaceGravityCenter = 0.0f;
    float minDistToFaceGravityCenter = 10.0f;
    
    for( int i = 0; i < numSpheres; ++i )
    {
        float lenght0 = length( mSpheresPositions[i].mPosition - cubeCorner0 );
        float lenght1 = length( mSpheresPositions[i].mPosition - cubeCorner1 );
        float lenght2 = length( mSpheresPositions[i].mPosition - cubeCorner2 );
        float lenght3 = length( mSpheresPositions[i].mPosition - cubeCorner3 );
        
        int face = 4;
        int color = 4;
        vec3 cubeCorner = vec3( 0.0f );
        float distToCorner = 0.0f;
        int sphereType = 0;
        vec2 tetraedreEdgeAdjacentFaces = vec2( -1, -1 );
        vec3 tetraedreCornerAdjacentFaces = vec3( -1, -1, -1 );
        vec3 faceNormal = vec3(0);
        float distToFaceGravityCenter = -1.0f;
        
        
        // Particles on a face
        if( lenght0 - lenght1 < 0.0f && lenght0 - lenght2 < 0.0f && lenght0 - lenght3 < 0.0f )
        {
            color = face = 0;
            cubeCorner = cubeCorner0;
            distToCorner = lenght0;
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            faceNormal = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
        }
        else if ( lenght1 - lenght0 < 0.0f && lenght1 - lenght2 < 0.0f && lenght1 - lenght3 < 0.0f )
        {
            color = face = 1;
            cubeCorner = cubeCorner1;
            distToCorner = lenght1;
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[0], mUnitCube[2] ) );
            faceNormal = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
        }
        else if ( lenght2 - lenght0 < 0.0f && lenght2 - lenght1 < 0.0f && lenght2 - lenght3 < 0.0f )
        {
            color = face = 2;
            cubeCorner = cubeCorner2;
            distToCorner = lenght2;
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[0], mUnitCube[7], mUnitCube[5] ) );
            faceNormal = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
        }
        else if ( lenght3 - lenght0 < 0.0f && lenght3 - lenght1 < 0.0f && lenght3 - lenght2 < 0.0f )
        {
            color = face = 3;
            cubeCorner = cubeCorner3;
            distToCorner = lenght3;
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[2], mUnitCube[5] ) );
            faceNormal = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
        }
        // Particles on a edge
        else if( lenght3 - lenght0 == 0.0f && lenght3 - lenght1 < 0.0f && lenght3 - lenght2 < 0.0f )
        {
            distToCorner = lenght0;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 0, 3 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            faceNormal = normalize( faceNormal0 + faceNormal3 );
            color = 5;
            face = -1;
        }
        else if( lenght2 - lenght0 == 0.0f && lenght2 - lenght1 < 0.0f && lenght2 - lenght3 < 0.0f )
        {
            distToCorner = lenght0;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 0, 2 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            faceNormal = normalize( faceNormal0 + faceNormal2 );
            color = 5;
            face = -1;
        }
        else if( lenght1 - lenght0 == 0.0f && lenght1 - lenght2 < 0.0f && lenght1 - lenght3 < 0.0f )
        {
            distToCorner = lenght0;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 0, 1 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            faceNormal = normalize( faceNormal0 + faceNormal1 );
            color = 5;
            face = -1;
        }
        else if( lenght1 - lenght0 < 0.0f && lenght1 - lenght2 == 0.0f && lenght1 - lenght3 < 0.0f )
        {
            distToCorner = lenght1;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 2, 1 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[0], mUnitCube[2] ) );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            faceNormal = normalize( faceNormal1 + faceNormal2 );
            // in this case faceNormal is parallel to the rotationAlign axe, we divide the rotation in 2 subrotations
            mSpheresPositions[i].mSubNormal1 = faceNormal1;
            color = 5;
            face = -1;
        }
        else if( lenght1 - lenght0 < 0.0f && lenght1 - lenght2 < 0.0f && lenght1 - lenght3 == 0.0f )
        {
            distToCorner = lenght1;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 3, 1 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[2], mUnitCube[5] ) );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            faceNormal = normalize( faceNormal1 + faceNormal3 );
            color = 5;
            face = -1;
        }
        else if( lenght2 - lenght0 < 0.0f && lenght2 - lenght1 < 0.0f && lenght2 - lenght3 == 0.0f )
        {
            distToCorner = lenght2;
            sphereType = 1;
            tetraedreEdgeAdjacentFaces = vec2( 3, 2 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[2], mUnitCube[5] ) );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            faceNormal = normalize( faceNormal2 + faceNormal3 );
            color = 5;
            face = -1;
        }
        // Particles on a corner
        else if( lenght2 - lenght0 == 0.0f && lenght2 - lenght1 == 0.0f && lenght2 - lenght3 <= 0.0f )
        {
            distToCorner = lenght0;
            sphereType = 2;
            tetraedreCornerAdjacentFaces = vec3( 0, 1, 2 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            faceNormal = normalize( faceNormal0 + faceNormal1 + faceNormal2 );
            color = 5;
        }
        else if( lenght2 - lenght0 == 0.0f && lenght2 - lenght1 < 0.0f && lenght2 - lenght3 == 0.0f )
        {
            distToCorner = lenght0;
            sphereType = 2;
            tetraedreCornerAdjacentFaces = vec3( 0, 2, 3 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            faceNormal = normalize( faceNormal0 + faceNormal2 + faceNormal3 );
            color = 5;
        }
        else if( lenght2 - lenght0 < 0.0f && lenght2 - lenght1 == 0.0f && lenght2 - lenght3 == 0.0f )
        {
            distToCorner = lenght1;
            sphereType = 2;
            tetraedreCornerAdjacentFaces = vec3( 1, 2, 3 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[0], mUnitCube[2] ) );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            vec3 faceNormal2 = computeTriangleNormal( mUnitCube[0], mUnitCube[7], mUnitCube[5] );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            faceNormal = normalize( faceNormal1 + faceNormal2 + faceNormal3 );
            color = 5;
        }
        else if( lenght1 - lenght0 == 0.0f && lenght1 - lenght2 < 0.0f && lenght1 - lenght3 ==  0.0f )
        {
            distToCorner = lenght0;
            sphereType = 2;
            tetraedreCornerAdjacentFaces = vec3( 0, 1, 3 );
            distToFaceGravityCenter = length( mSpheresPositions[i].mPosition - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
            vec3 faceNormal0 = computeTriangleNormal( mUnitCube[2], mUnitCube[0], mUnitCube[5] );
            vec3 faceNormal1 = computeTriangleNormal( mUnitCube[7], mUnitCube[0], mUnitCube[2] );
            vec3 faceNormal3 = computeTriangleNormal( mUnitCube[7], mUnitCube[2], mUnitCube[5] );
            faceNormal = normalize( faceNormal0 + faceNormal1 + faceNormal3 );
            color = 5;
        }
        
        if ( maxDistToFaceGravityCenter < distToFaceGravityCenter ) maxDistToFaceGravityCenter = distToFaceGravityCenter;
        if ( minDistToFaceGravityCenter > distToFaceGravityCenter ) minDistToFaceGravityCenter = distToFaceGravityCenter;
        
        mSpheresPositions[i].mTetraedreFace = face;
        mSpheresPositions[i].mColor = color;
        mSpheresPositions[i].mCubeOppositeCornerPosition = cubeCorner;
        mSpheresPositions[i].mDist_To_Cormer = distToCorner;
        mSpheresPositions[i].mSphereType = sphereType;
        mSpheresPositions[i].mTetraedreEdgeAdjacentFaces = tetraedreEdgeAdjacentFaces;
        mSpheresPositions[i].mTetraedreCornerAdjacentFaces = tetraedreCornerAdjacentFaces;
        mSpheresPositions[i].mNormal = faceNormal;
        mSpheresPositions[i].mDistToFaceGravityCenter = distToFaceGravityCenter;
    }
    console() << " maxDistToFaceGravityCenter " << maxDistToFaceGravityCenter << std::endl;
    console() << " minDistToFaceGravityCenter " << minDistToFaceGravityCenter << std::endl;
    
    // We need mDistToFaceGravityCenter between 0 and 1
    // -> conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // minDistToFaceGravityCenter - maxDistToFaceGravityCenter  ->  0 - 1
    for( int i = 0; i < numSpheres; ++i )
    {
        float convertedDist = ( (mSpheresPositions[i].mDistToFaceGravityCenter - minDistToFaceGravityCenter )
                               / ( maxDistToFaceGravityCenter - minDistToFaceGravityCenter) );
        if ( mSpheresPositions[i].mDistToFaceGravityCenter == -1.0f ) mSpheresPositions[i].mDistToFaceGravityCenter = 0.0f;
        else mSpheresPositions[i].mDistToFaceGravityCenter = 1.0f - convertedDist;
    }
    
    
}

// Chose arbitrary a corner and his opposite base triangle from the tetraedre
// Ex: Top Corner = mUnitCube[5] -> opposite triangle = Face 4 ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
void hoofeli03App::computeSpheresLines( const vec3 &topCorner, const uint16_t oppositeFace, const vec3 &baseTrianlePoint1, const vec3 &baseTrianlePoint2, const vec3 &baseTrianlePoint3 )
{
    vec3 gravityCenter = computeTriangleGravitycenter( baseTrianlePoint1, baseTrianlePoint2, baseTrianlePoint3 );
    
    // Compute the number of lines made of spheres
    // S the number of subdivions and NL the number of lines
    //
    // S = 1          * P1                S = n               if Sphere in triangles          * P1
    // NL = 3                             NL = 2^n + 1        NL = 2^n + 2^n * 2
    //                                                                                        *
    //            *       *                                                               *       *
    //                                                                                        *
    //                                                                                    *
    //        *       *       *                                                       *       *       *
    //        P2              P3                                                      P2              P3
    //
    
    uint16_t NL;
    NL = pow( 2, mSubdivisions ) + 2 * pow( 2, mSubdivisions );
    console() << " mSubdivisions " << mSubdivisions << std::endl;
    
    // Now compute the number of segmnents and their length by dividing the tetraedre height with NL - 1
    float segmentsLength = length( gravityCenter - topCorner ) / ( NL - 1 );
    
    console() << " NL " << NL << std::endl;
    console() << " length( gravityCenter - topCorner ) " << length( gravityCenter - topCorner ) << std::endl;
    console() << " segmentsLength " << segmentsLength << std::endl;
    
    // for testing only
    //uint16_t maxElement = 0;
    
    int numSpheres = mSpheresPositions.size();
    
    for( int i = 0; i < numSpheres; ++i )
    {
        // The height is represented by the segment [ corner, gravityCenter ]
        //
        //        * topCorner
        //        |
        //        | * cubeCorner3
        //        * gravityCenter of (cubeCorner0, cubeCorner2, cubeCorner3)
        // *             *
        // cubeCorner0   cubeCorner2
        //
        // tetraedre height :segment [ topCorner, gravityCenter ]
        //
        
        mSpheresPositions[i].mDist_Projected_Position_To_Corner = 2.0f;
        
        // step 1 for all face != face( cubeCorner0, cubeCorner2, cubeCorner3 ) which is the opposite face off the topCorner
        if ( mSpheresPositions[i].mTetraedreFace != oppositeFace )
        {
            // First compute shortest distance from a point x0 on a face to tetraedre height
            mSpheresPositions[i].mDist_To_Tetraedre_Heigth = computePointSegDist( mSpheresPositions[i].mPosition, topCorner, gravityCenter );
            
            // Then compute the distance d between the projection of x0 on the height and the top corner
            //
            //                                * topCorner                  P0
            //                                |                            *
            //                                |  d                         |\
            //                                |                            | \         Phytagore:  a^2 + b^2 = c^2
            // projection of x0 on the heigth * -------- * x0             b|  \ c                  b^2 = c^2 - a^2
            //                                |                            |   \
            //                                |                            | a  \
            //                                * gravityCenter           P1 *-----* P2
            //
            
            float squaredDistToTopCormer = length2( mSpheresPositions[i].mPosition - topCorner );
            
            mSpheresPositions[i].mDist_Projected_Position_To_Corner = sqrt ( squaredDistToTopCormer - pow( mSpheresPositions[i].mDist_To_Tetraedre_Heigth, 2 ) );
            
            //mSpheresPositions[i].mPosition = topCorner + length( savepos - topCorner ) * normalize( savepos - topCorner );
            //mSpheresPositions[i].mPosition = topCorner - mSpheresPositions[i].mDist_Projected_Position_To_Corner * normalize( topCorner - gravityCenter );
            
        }
        else if ( mSpheresPositions[i].mTetraedreFace == oppositeFace )
        {
            // Just for testing
            //mSpheresPositions[i].mColor = 2;
        }
        
        // Now Compute the line Number of the Sphere
        for( int l = 0; l < NL; ++l )
        {
            if ( mSpheresPositions[i].mTetraedreFace != oppositeFace )
            {
                if ( mSpheresPositions[i].mDist_Projected_Position_To_Corner / segmentsLength < (float) l )
                {
                    
                    mSpheresPositions[i].mLineNumber = l;
                    
                    // Just for testing
                    /*if ( maxElement < l ) maxElement++;
                     
                     if ( l & 1 )
                     mSpheresPositions[i].mColor = 0;
                     //else
                     mSpheresPositions[i].mColor = 1;
                     */
                    break;
                }
            }
        }
    }
}


vec3 hoofeli03App::computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 )
{
    return vec3( point1.x + point2.x + point3.x, point1.y + point2.y + point3.y, point1.z + point2.z + point3.z ) / 3.0f;
}


// Rotate an object fron angle d and vector z => Inigo Quilez
mat4 hoofeli03App::rotationAlign( vec3 d, vec3 z )
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


// Compute shortest distance from point to line segment in 3D
float hoofeli03App::computePointSegDist( const vec3 &x0, const vec3 &x1, const vec3 &x2 )
{
    //    x1 o-----------------o x2
    //                  |
    //                  | d
    //                  |
    //                  o x0
    //
    //  d = norm( cross( x2 - x1, x1 - x0 ) ) / norm( x2 - x1 );
    
    return length( cross( x0 - x1, x0 - x2 ) ) / length( x2 - x1 );
}


// Position of the projected point p on a line (Not used now)
vec3 hoofeli03App::projectPointToSegment( const vec3 &x0, const vec3 &x1, const vec3 &x2 )
{
    //                  p
    //  (A) x1 o-----------------o x2 (B)
    //                  |
    //                  | d
    //                  |
    //                  o x0 (p)
    //
    
    // function [q] = project_point_to_line_segment(A,B,p)
    
    // vector from A to B
    vec3 AB = ( x2 - x1 );
    
    // squared distance from A to B
    float AB_squared = dot( AB, AB );
    
    if( AB_squared == 0 )
    {
        // A and B are the same point
        return x1;
    }
    else
    {
        // vector from A to p
        vec3 Ap = ( x0 - x1 );
        // from http://stackoverflow.ole/questions/849211/
        // Consider the line extending the segment, parameterized as A + t (B - A)
        // We find projection of point p onto the line.
        // It falls where t = [(p-A) . (B-A)] / |B-A|^2
        
        float t = dot( Ap, AB ) / AB_squared;
        
        if (t <= 0.0)
        {
            //"Before" A on the line, just return A
            return x1;
        }
        
        else if ( t >= 1.0 )
        {
            // "After" B on the line, just return B
            return x2;
        }
        else
        {
            // projection lines "inbetween" A and B on the line
            return x1 + t * AB;
        }
    }
}

vec3 hoofeli03App::computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 )
{
    vec3 e0 = pos2 - pos1;
    vec3 e1 = pos3 - pos1;
    
    return normalize( cross( e0, e1 ));
}


// Sort and remove duplicates indices
void hoofeli03App::sortIndicesCopy( const std::vector<uint16_t> &indices )
{
    size_t indicesSize = indices.size();
    
    for( uint16_t i = 0; i < indicesSize; ++i ) {
        mCheckedIndices.push_back( indices[i] );
    }
    
    // sort and remove doublons
    std::sort( mCheckedIndices.begin(), mCheckedIndices.end() );
    auto last = std::unique( mCheckedIndices.begin(), mCheckedIndices.end() );
    mCheckedIndices.erase( last, mCheckedIndices.end() );
}


// Remove duplicates indices without sorting
void hoofeli03App::uniqueVertexsCopy( const std::vector<vec3> &vertexPositions )
{
    for ( std::vector<vec3>::const_iterator it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
    {
        if ( std::find( mUniquePosCoords.begin(), mUniquePosCoords.end(), *it ) == mUniquePosCoords.end() )
        {
            mUniquePosCoords.push_back( *it );
        }
    }
}


void hoofeli03App::subdivide( int mSubdivision )
{
    for( int j = 0; j < mSubdivision; ++j ) {
        const size_t numTriangles = mIndices.size() / 3;
        for( uint16_t i = 0; i < numTriangles; ++i ) {
            uint16_t index0 = mIndices[i * 3 + 0];
            uint16_t index1 = mIndices[i * 3 + 1];
            uint16_t index2 = mIndices[i * 3 + 2];
            
            uint16_t index3 = (uint16_t)mPosCoords.size();
            uint16_t index4 = index3 + 1;
            uint16_t index5 = index4 + 1;
            
            // add new triangles
            mIndices[i * 3 + 1] = index3;
            mIndices[i * 3 + 2] = index5;
            
            mIndices.push_back( index3 );
            mIndices.push_back( index1 );
            mIndices.push_back( index4 );
            
            mIndices.push_back( index5 );
            mIndices.push_back( index3 );
            mIndices.push_back( index4 );
            
            mIndices.push_back( index5 );
            mIndices.push_back( index4 );
            mIndices.push_back( index2 );
            
            // add new positions
            mPosCoords.push_back( 0.5f * ( mPosCoords[index0] + mPosCoords[index1]) );
            mPosCoords.push_back( 0.5f * ( mPosCoords[index1] + mPosCoords[index2]) );
            mPosCoords.push_back( 0.5f * ( mPosCoords[index2] + mPosCoords[index0]) );
        }
    }
}


void hoofeli03App::setup()
{
    //gl::enableVerticalSync( true );
    
    // Setup animation variables
    mSurroundingTetraedreMove       = 0;
    mSurroundingTetraedreMoveDist   = 0.0f;
    mCounter_1                      = 0.0f;
    
    // Freq
    mFreq       = 1.0f;
    mFreq2      = 1.0f;
    mWaveCoef   = 0.122f;
    
    // PBR
    mMetallic   = 1.00f;
    mRoughness  = 0.40f;
    
    // God Rays
    mExposure   = 0.1f;
    mDecay      = 0.98f;
    mDensity    = 0.13f;
    mWeight     = 0.01f;
    
    // Bloom
    aBloomValues[0] = 0.227027f;
    aBloomValues[1] = 0.1945946f;
    aBloomValues[2] = 0.1216216f;
    aBloomValues[3] = 0.054054f;
    aBloomValues[4] = 0.016216f;
    
    // HDR
    mMixBloomGodrays        = 1.0f;
    mHdrExposure            = 1.9f;
    mHdrGamma               = 1.3f;
    mChromaticAberration    = 0.0f;
    
    // Drawing
    mMode       = 2;
    mRedCoef    = 0.0f;
    mSizeCoef   = 0.0f;
    mColorIndex = 1.0f;
    
    // Ligthdata
    mLightCamera.distanceRadius     = 20.0f;
    mLightCamera.viewpoint          = vec3( mLightCamera.distanceRadius );
    mLightCamera.fov                = 60.0f;
    mLightCamera.target             = vec3( 0 );
    mLightCamera.toggleViewpoint	= false;
    mDistanceConverstion            = 10.4f;
    
    // ReactiveCam
    mReactiveCam.set( 7.0f, mCameraTarget, 0.01f, 10.0f );
    mLightCamera.camera.setPerspective( mLightCamera.fov, getWindowAspectRatio(), 0.01, 10.0 ); // 1000
    mReactiveCam.setKeyEvent( mKeyPressed );
    mIsBeat = false;

    // Transitions
    mStart_Transition0 = false;
    mStart_Transition1 = false;
    mStart_Transition2 = false;
    mStart_Transition3 = false;
    
    // Load and compile the shaders
    loadShaders();
    
    // Load textures
    loadTextures();
    
    //createGridTest();
    
    // Create the meshes.
    createGeometry();
    
    // Init instancied particles
    mInstanciedParticles.setup( &(mReactiveCam.getCamera()), &mLightCamera.camera, mSpheresPositions, 2 );

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
            case KeyEvent::KEY_1 :
                mStart_Transition0 = true; mStart_Transition1 = false;
                mStart_Transition2 = false; mStart_Transition3 = false;
                break;
            case KeyEvent::KEY_2 :
                mStart_Transition0 = false; mStart_Transition1 = true;
                mStart_Transition2 = false; mStart_Transition3 = false;
                break;
            case KeyEvent::KEY_3 :
                mStart_Transition0 = false; mStart_Transition1 = false;
                mStart_Transition2 = true; mStart_Transition3 = false;
                break;
            case KeyEvent::KEY_4 :
                mStart_Transition0 = false; mStart_Transition1 = false;
                mStart_Transition2 = false; mStart_Transition3 = true;
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
                    mXMLcom.start( "audio.xml" );
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
    if ( XML == TRUE ) mXMLcom.start( "audio.xml" );
}


void hoofeli03App::update()
{
    // Scene rotation
    mSceneRotation *= rotate( 0.005f, vec3( 0.5f, -1.5f, 0.5f )  );
    //mSceneRotation = glm::mat4( 1.0f );
    
    // Update Camera
    mReactiveCam.update( mIsBeat, normalize( vec3( 0.5f, -1.5f, 0.5f ) ), true, 0.0f );
    mLightCamera.viewpoint = mLightCamera.distanceRadius * normalize( mReactiveCam.getPosition() ) * -1.0f;
    mLightCamera.camera.lookAt( mLightCamera.viewpoint, mLightCamera.target );
    mIsBeat = false;
    
    // Update instanced particles
    mInstanciedParticles.update( mFreq, mFreq2, mWaveCoef, mSizeCoef, mMode, mSceneRotation, mFboSize, mCounter_1 );
    
    // Update animation variables
    if( mSurroundingTetraedreMove == 1 && mSurroundingTetraedreMoveDist < 4.0f )
    {
        mSurroundingTetraedreMoveDist += 0.02f;
    }
    else if( mSurroundingTetraedreMove == 2 && mSurroundingTetraedreMoveDist > 0.0f )
    {
        mSurroundingTetraedreMoveDist -= 0.02f;
    }
    if( mSurroundingTetraedreMoveDist < 0.0f ) mSurroundingTetraedreMoveDist = 0.0f;

    // Transitions
    transition_0( mStart_Transition0 );
    transition_1( mStart_Transition1 );
    transition_2( mStart_Transition2 );
    transition_3( mStart_Transition3 );
    
    // COM
    if ( mPlayXML == TRUE )
    {
        //if ( mDistanceConverstionOut > mDistanceConverstion ) mDistanceConverstionOut -= 0.1f;
        mXMLcom.update();
        setXMLAnimationPhases();
    }
    
    // Save render fbo
    if ( mSaveScreen == true && mXMLcom.isLooping() == true ){
        saveImage();
    }
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
    
    // 30 fps
    //mCounter_1 += 0.03333f;
    mCurrentFrame++;
    
    //createGridTest();
}


void hoofeli03App::draw()
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
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
            };
            gl::drawBuffers( 3, buffers );
        }
        
        gl::clear();
        
        // Set Matrices
        gl::setMatrices( mReactiveCam.getCameraMatrix() );

        gl::ScopedModelMatrix modelScope;
        
        // Draw the grid.
        /*if( mGrid ) {
         mGrid->draw();
         // draw the coordinate frame with length 2.
         gl::drawCoordinateFrame( 2 );
         }*/
        
        // Render Frontside only
        gl::ScopedFaceCulling cullScope( mEnableFaceFulling, GL_BACK );
        gl::cullFace( GL_BACK );
        
        // Render Tetraedre
        {
            // Rotate it slowly around the y-axis.
            gl::ScopedModelMatrix matScope;
            gl::multModelMatrix( mSceneRotation );
            gl::ScopedTextureBind scopedTextureBind0( mTextureSpectrum, 0 );
            mPrimitive->getGlslProg()->uniform( "uLineStyle", 1 );
            mPrimitive->getGlslProg()->uniform( "uColor", vec3( 0.0f, 0.0f, 0.0f ) );
            mPrimitive->draw();
        }
        
        if ( mSurroundingTetraedreMoveDist <= 4.0f ){
            // Rotate it slowly around the y-axis.
            //gl::ScopedModelMatrix matScope;
            //gl::multModelMatrix( mSceneRotation );
            gl::ScopedTextureBind scopedTextureBind0( mTextureSpectrum, 0 );
            gl::ScopedTextureBind scopedTextureBind1( mTextureAlbedo, 1 );
            gl::ScopedTextureBind scopedTextureBind2( mTextureNormal, 2 );
            gl::ScopedTextureBind scopedTextureBind3( mTextureRoughness, 3 );
            gl::ScopedTextureBind scopedTextureBind4( mTextureLine, 4 );
            gl::ScopedTextureBind scopedTextureBind5( mTextureLigth, 5 );
            mPhongShader->uniform( "uTexSpectrum", 0 );
            mPhongShader->uniform( "uTexAlbedo", 1 );
            mPhongShader->uniform( "uTexNormal", 2 );
            mPhongShader->uniform( "uTexRoughness", 3 );
            mPhongShader->uniform( "uTexLine", 4 );
            mPhongShader->uniform( "uTextureLigth", 5 );
            mPhongShader->uniform( "uLightPosition", mat3( mReactiveCam.getViewMatrix() ) * mReactiveCam.getPosition() );
            mPhongShader->uniform( "uTime", 0.1f * mCounter_1 );
            mPhongShader->uniform( "uWorldtoLightMatrix", mLightCamera.camera.getViewMatrix() );
            mPhongShader->uniform( "uDistanceConverstion", 10.40f );
            if ( mXMLcom.mStep == 0 ) mPhongShader->uniform( "uEnergy", mXMLcom.mEnergy1 * clamp( 1.0f - 5.0f * mSurroundingTetraedreMoveDist, 0.0f, 1.0f ) );
            else mPhongShader->uniform( "uEnergy", 0.3f * clamp( 1.0f - 5.0f * mSurroundingTetraedreMoveDist, 0.0f, 1.0f ) );
            glm::mat4 modelMatrix = glm::mat4( 1.0f );
            modelMatrix = modelMatrix * mSceneRotation;
            modelMatrix = glm::translate( modelMatrix, mSurroundingTetraedreMoveDist * normalize( mUnitCube[6] ) );
            gl::setModelMatrix( modelMatrix );
            mSurroundingTetraedres[0]->draw();
            modelMatrix = glm::mat4( 1.0f );
            modelMatrix = modelMatrix * mSceneRotation;
            modelMatrix = glm::translate( modelMatrix, mSurroundingTetraedreMoveDist * normalize( mUnitCube[1] ) );
            gl::setModelMatrix( modelMatrix );
            mSurroundingTetraedres[1]->draw();
            modelMatrix = glm::mat4( 1.0f );
            modelMatrix = modelMatrix * mSceneRotation;
            modelMatrix = glm::translate( modelMatrix, mSurroundingTetraedreMoveDist * normalize( mUnitCube[4] ) );
            gl::setModelMatrix( modelMatrix );
            mSurroundingTetraedres[2]->draw();
            modelMatrix = glm::mat4( 1.0f );
            modelMatrix = modelMatrix * mSceneRotation;
            modelMatrix = glm::translate( modelMatrix, mSurroundingTetraedreMoveDist * normalize( mUnitCube[3] ) );
            gl::setModelMatrix( modelMatrix );
            mSurroundingTetraedres[3]->draw();
         
        }
        // Render Particles
        {
            mInstanciedParticles.drawInstanced( mDistanceConverstion, mMetallic, mRoughness, mMode, mRedCoef, mColorIndex );
        }
        
    }
    
    // GodRays
    {
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
    }
    
    // Bloom
    // Ping pong bloom vars
    bool horizontal = true;
    bool first_iteration = true;
    int numberOfPasses = 6;
    
    {
     /*   const gl::ScopedFramebuffer scopedFrameBuffer( mBloomFBO );
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
                mTexturemRender[ 2 ]->bind(0);
                //mTexturemRender[ 1 ]->bind(0);
            }
            else mTextureBloom[(int)!horizontal]->bind(0);
            
            mBatchBloomRect->getGlslProg()->uniform( "uWeight", aBloomValues, 5 );;
            mBatchBloomRect->getGlslProg()->uniform( "uHorizontal", horizontal );
            mBatchBloomRect->getGlslProg()->uniform( "uWidth", height );
            mBatchBloomRect->getGlslProg()->uniform( "uHeight", width );
            mBatchBloomRect->draw();
            
            if ( first_iteration ) mTexturemRender[2]->unbind(0);
            else mTextureBloom[(int)!horizontal]->unbind(0);
            
            horizontal = !horizontal;
            if ( first_iteration )
                first_iteration = false;
        }
      */
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
        const gl::ScopedTextureBind scopedTextureBind1( mTextureBloom[(int)!horizontal], 0 ); // bloom texture
        const gl::ScopedTextureBind scopedTextureBind0( mTextureGodRays[ 0 ], 1 ); // Godrays
        const gl::ScopedTextureBind scopedTextureBind2( mTexturemRender[ 0 ], 2 ); // Scene
        mBatchHdrRect->getGlslProg()->uniform( "uIsBloom", false );
        mBatchHdrRect->getGlslProg()->uniform( "uIsGodrays", true );
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
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 4 ], 0 );
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureGodRays[ 0 ], 0 );
        mBatchToScreenRect->draw();
    }
    
    // Disable the depth buffer.
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    // Render the parameter windows.
    if( mParams ) {
        mParams->draw();
    }
}


void hoofeli03App::resize()
{
    mLightCamera.camera.setAspectRatio( mFboSize.x/mFboSize.y );
    mReactiveCam.resize( vec2( mFboSize.x, mFboSize.y ) );
    
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
    
    // Bloom FBO
    {
        // must adjust size to half the screen
        ivec2 sz = ivec2( w, h );
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


void hoofeli03App::createGeometry()
{
    // Create center Tetraedre to draw
    mPrimitive = gl::Batch::create( createPrimitiveFromCube(), mWireframeShader );
    mPrimitive->getGlslProg()->uniform( "uTexSpectrum", 0 );
    
    // Create WirePrimitiveFromCube to generate spheres positions, we do not draw it
    mPrimitiveSubdivided = gl::Batch::create( createWirePrimitiveFromCube(), mWireframeShader );
    
    // Create the Tetraedres surrounding the center Tetraedre
    //mSurroundingTetraedres[0] = gl::Batch::create( gl::VboMesh::create( geom::Teapot() ), mPhongShader );
    
    mSurroundingTetraedres[0] = gl::Batch::create( createPrimitiveFromPoints( mUnitCube[6], mUnitCube[2], mUnitCube[5], mUnitCube[7] ), mPhongShader );
    mSurroundingTetraedres[1] = gl::Batch::create( createPrimitiveFromPoints( mUnitCube[1], mUnitCube[0], mUnitCube[5], mUnitCube[2] ), mPhongShader );
    mSurroundingTetraedres[2] = gl::Batch::create( createPrimitiveFromPoints( mUnitCube[4], mUnitCube[5], mUnitCube[0], mUnitCube[7] ), mPhongShader );
    mSurroundingTetraedres[3] = gl::Batch::create( createPrimitiveFromPoints( mUnitCube[3], mUnitCube[7], mUnitCube[0], mUnitCube[2] ), mPhongShader );
    
    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );

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


void hoofeli03App::createGridTest()
{
    mGrid = gl::VertBatch::create( GL_LINES );
    mGrid->begin( GL_LINES );
    mGrid->color( Color( 1.0f, 1.0f, 1.0f ) );
    mGrid->color( Color( 1.0f, 1.0f, 1.0f ) );
    
    mGrid->vertex( mGridP0 );
    mGrid->vertex( mGridP1 );
    
    mGrid->end();
}


void hoofeli03App::loadTextures()
{
    // Load the textures.
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    mTextureSpectrum = gl::Texture::create( loadImage( loadAsset( "textures/spectrumM3.png" ) ), fmt );
    mTextureLigth = gl::Texture::create( loadImage( loadAsset( "textures/ligthTexture.png" ) ), fmt );
    fmt.setWrap( GL_REPEAT, GL_REPEAT );
    mTextureStripes = gl::Texture::create( loadImage( loadAsset( "textures/stripes.jpg" ) ), fmt );
    mTextureAlbedo = gl::Texture::create( loadImage( loadAsset( "textures/albedo.png" ) ), fmt );
    mTextureNormal = gl::Texture::create( loadImage( loadAsset( "textures/normal-ogl.png" ) ), fmt );
    mTextureRoughness = gl::Texture::create( loadImage( loadAsset( "textures/roughness.png" ) ), fmt );
    mTextureLine = gl::Texture::create( loadImage( loadAsset( "textures/line.png" ) ), fmt );
}


// Save renderFbo
void hoofeli03App::saveImage()
{
    // Pull down the current window as a surface and pass it to writeImage
    writeImage( getHomeDirectory() / "deformedSphere" / ( toString(1) + "_" + toString( mCurrentFrame ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
}


void hoofeli03App::loadShaders()
{
    try {
        mPhongShader = gl::GlslProg::create( loadAsset( "phong.vert" ), loadAsset( "phong.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Phong shader: " << exc.what() );
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
}


void hoofeli03App::createParams()
{
    mParams = params::InterfaceGl::create( getWindow(), "Geometry Demo", toPixels( ivec2( 300, 400 ) ) );
    mParams->setOptions( "", "valueswidth=100 refresh=0.1" );
    mParams->addSeparator();
    mParams->addParam( "Face Culling", &mEnableFaceFulling ).updateFn( [this] { gl::enableFaceCulling( mEnableFaceFulling ); } );
    mParams->addSeparator();
    mParams->addParam( "Freq", &mFreq, "min=-0.0 max=100.0 step=0.1 keyIncr=q keyDecr=w" );
    mParams->addParam( "Freq2", &mFreq2, "min=-0.0 max=10000.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "WaveCoef", &mWaveCoef, "min=-0.0 max=100.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "SizeCoef", &mSizeCoef, "min=-0.0 max=1.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "distance ligth", &mLightCamera.distanceRadius ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "distance converstion", &mDistanceConverstion ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "mRedCoef", &mRedCoef ).min( 0.0f ).step( 0.001f);
    mParams->addSeparator();
    mParams->addParam( "PBR Metallic", &mMetallic ).min( 0.0f ).max( 1.0f ).step( 0.01f);
    mParams->addParam( "PBR Roughness", &mRoughness ).min( 0.0f ).max( 1.0f ).step( 0.01f);
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
    mParams->addParam( "Drawing Mode", &mMode, "min=0.0 max=4.0 step=1.0 keyIncr=q keyDecr=w" );
    mParams->addParam( "Color Index", &mColorIndex, "min=0.0 max=4.0 step=1.0 keyIncr=q keyDecr=w" );
    mParams->addSeparator();
    mParams->addParam( "Axis X Rot speed", &mReactiveCam.mCamParams.mAxisXRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Axis Y Rot speed", &mReactiveCam.mCamParams.mAxisYRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotation speed", &mReactiveCam.mCamParams.mRotationSpeed, "min=-1.0 max=1.0 step=0.001 keyIncr=q keyDecr=w" );
}


void hoofeli03App::setXMLAnimationPhases()
{
    if ( mXMLcom.mStep == 0 )
    {
        float energy1 = pow( mXMLcom.mEnergy1, 2.0f );
        mDensity = energy1;
        mExposure = mXMLcom.mEnergy1;
        mHdrGamma = 1.3 + energy1*0.8f;
    }
    else if ( mXMLcom.mStep == 1 )
    {
        mFreq = 2.5f;
        
        mReactiveCam.mCamParams.mRotationSpeed = 0.025f;
        
        if ( mStart_Transition0 == false ) mStart_Transition0 = true;

        if ( mHdrGammaCounter < 0.6f ) mHdrGammaCounter += 0.1f;


        if( mXMLcom.mIsBeat0 == true )
        {
            if ( mEnergyCoef_1 < 1.0f ) mEnergyCoef_1 += 0.02f;
            mbeatCounter_1 = 0.0f;
            mbeatCounter_2 = 0.0f;
        }
        
        mWaveCoef = ( 0.2f + 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) )) * 0.7f * mEnergyCoef_1;
        if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.005 ;
        
        mHdrGamma = mHdrGammaCounter + ( 1.0 - easeOutExpo( clamp( mbeatCounter_2, 0.0f, 1.0f ) )) * 1.0f;
        if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02;

        if( mReactiveCam.getCameraDist() > 5.5f ) mReactiveCam.setCameraDist( mReactiveCam.getCameraDist() - 0.004f );
        if( mReactiveCam.getCameraDist() < 5.5f ) mReactiveCam.setCameraDist( 5.5f );
    }
    else if ( mXMLcom.mStep == 2 )
    {
        mReactiveCam.mCamParams.mRotationSpeed = 0.03f;
        
        if( mXMLcom.mIsBeat0 == true )
        {
            if ( mEnergyCoef_1 < 1.0f ) mEnergyCoef_1 += 0.02f;
            mbeatCounter_1 = 0.0f;
            mbeatCounter_2 = 0.0f;
            mIsBeat = true;
        }
        
        mWaveCoef = ( 0.2f +  1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) )) * 0.7f * mEnergyCoef_1;
        if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.005 ;
        
        mHdrGamma = 0.6f + ( 1.0 - easeOutExpo( clamp( mbeatCounter_2, 0.0f, 1.0f ) )) * 1.0f;
        if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02;
        
        mDistanceConverstion = 10.4 + mXMLcom.mRangeEnergy * 0.4f;
    }
    else if ( mXMLcom.mStep == 3 )
    {
        mFreq = 8.5f;
        mWaveCoef = 0.2f + mXMLcom.mEnergy2;
        mHdrGamma = 0.6f + 2.0f * mXMLcom.mEnergy2;
        mChromaticAberration = clamp( 0.03f * mXMLcom.mRangeEnergy, 0.0f, 0.0025f);
        mDistanceConverstion = 10.35;
        mDecay = 0.98f;

        if( mReactiveCam.getCameraDist() < 6.0f ) mReactiveCam.setCameraDist( mReactiveCam.getCameraDist() + 0.0013f );
        if( mReactiveCam.getCameraDist() > 6.0f ) mReactiveCam.setCameraDist( 6.0f );
    }
    else if ( mXMLcom.mStep == 4 )
    {
        mChromaticAberration = 0.0f;

        //mReactiveCam.mCamParams.mRotationSpeedCoef = 0.5f;
        mReactiveCam.mCamParams.mRotationSpeed = 0.035f;
        
        mStart_Transition0 = false;
        if ( mStart_Transition1 == false ) mStart_Transition1 = true;

        if( mXMLcom.mIsBeat0 == true )
        {
            //if ( mEnergyCoef_1 < 1.0f ) mEnergyCoef_1 += 0.02f;
            mbeatCounter_1 = 0.0f;
            mbeatCounter_2 = 0.0f;
            mIsBeat = true;
        }
        
        mWaveCoef = ( 0.25f + 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) )) * 1.1f;

        if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.015 ;
        
        mHdrGamma = 0.6f + ( 1.0 - easeOutExpo( clamp( mbeatCounter_2, 0.0f, 1.0f ) )) * 1.0f;
        if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02;
        
        mDistanceConverstion = 10.3 + mXMLcom.mRangeEnergy * 0.45f;

    }
    else if ( mXMLcom.mStep == 5 )
    {
        mFreq = 8.5f;
        mWaveCoef = 0.2f + mXMLcom.mEnergy2;
        mHdrGamma = 0.6f + 2.0f * mXMLcom.mEnergy2;
        mDistanceConverstion = 10.4;
        mChromaticAberration = clamp( 0.01f * mXMLcom.mRangeEnergy, 0.0f, 0.0045f);
    }
    else if ( mXMLcom.mStep == 6 )
    {
        mDistanceConverstion = 10.4 + mXMLcom.mRangeEnergy * 0.5f;
        if ( mXMLcom.mReadTime < 190.264313f )
        {
            mDecay = 0.97f;
            if( mReactiveCam.getCameraDist() < 7.0f ) mReactiveCam.setCameraDist( mReactiveCam.getCameraDist() + 0.0013f );
            if( mReactiveCam.getCameraDist() > 7.0f ) mReactiveCam.setCameraDist( 7.0f );
        
            //mReactiveCam.mCamParams.mRotationSpeedCoef = 0.5f;
            mReactiveCam.mCamParams.mRotationSpeed = 0.035f;
        
            mStart_Transition1 = false;
            if ( mStart_Transition2 == false ) mStart_Transition2 = true;
        
            mChromaticAberration = clamp( 0.02f * mXMLcom.mRangeEnergy, 0.0f, 0.0045f);

            if( mXMLcom.mIsBeat0 == true )
            {
                mbeatCounter_1 = 0.0f;
                mbeatCounter_2 = 0.0f;
                mIsBeat = true;
            }
        
            mWaveCoef = 0.122f + ( 2.0f * mXMLcom.mEnergy2 + ( 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) ))) * 0.12f;
            if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.01 ;
            
            mHdrGamma = 0.6f + ( 1.0 - easeOutExpo( clamp( mbeatCounter_2, 0.0f, 1.0f ) )) * 1.0f;
            if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02;
            
            mDistanceConverstion = 10.3 + mXMLcom.mRangeEnergy * 0.9f;

        }
        else if ( mXMLcom.mReadTime >= 190.264313f )
        {
            mDecay = 0.97f;
            //app::console() << "218.673920 " << std::endl;
            //app::console() << "mXMLcom.mReadTime " << mXMLcom.mReadTime << std::endl;

            mStart_Transition2 = false;
            
            if ( mStart_Transition3 == false ) mStart_Transition3 = true;
            if( mXMLcom.mIsBeat0 == true )
            {
                mbeatCounter_1 = 0.0f;
                mbeatCounter_2 = 0.0f;
                mIsBeat = true;
            }
            if ( mbeatCounter_3 > 0.0f ) mWaveCoef =  mWaveCounter + ( 2.0f * mXMLcom.mEnergy2 + ( 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) ))) * 0.15f * mbeatCounter_3;
            else if ( mbeatCounter_3 == 0.0f ) mWaveCoef =  mWaveCounter;

            if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.01f;
            
            /*app::console() << "( mXMLcom.mEnergy2 ) * mbeatCounter_3  " << ( mXMLcom.mEnergy2 ) << std::endl;
            app::console() << "mbeatCounter_1  " << ( 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) )) * 0.2f << std::endl;
            app::console() << "mWaveCounter  " << mWaveCounter << std::endl;
            app::console() << "mbeatCounter_3  " << mbeatCounter_3 << std::endl;*/

            
            mChromaticAberration = clamp( 0.02f * mXMLcom.mRangeEnergy, 0.0f, 0.0045f);
            float energy1 = mXMLcom.mRangeEnergy * 6.0f;
            float energy2 = mXMLcom.mRangeEnergy;
            mHdrGamma = 1.3 + energy2 * 0.8f;
            mDistanceConverstion = 10.3 + energy1;

            if ( mbeatCounter_2 < 1.0f ) mbeatCounter_2 += 0.02;
            
            // frame at 60fps to close: 200 => 3,20 sec
            // max mXMLcom.mReadTime = 228.668137 sec
            // so we  choose to close at 228.668137 - 10 sec = 228.668137 - 218.668137 => mXMLcom.mReadTime = 218.673920
            if ( mXMLcom.mReadTime >= 218.673920 )
            {

                float energy1 = mXMLcom.mRangeEnergy * 6.0f;
                float energy2 = pow( mXMLcom.mEnergy2, 2.0f );
                app::console() << "( energy1 " <<  energy1 << std::endl;
                mDistanceConverstion = 10.3 + energy1;
                mChromaticAberration = clamp( 0.01f * mXMLcom.mEnergy2, 0.0f, 0.0045f);
                mSurroundingTetraedreMove = 2;
                mExposure = 0.10f + energy1;

                if ( mFreq2 > 0.0f ) mFreq2 -= 0.005f;
                if ( mFreq2 < 0.0f ) mFreq2 = 0.0f;
                
                if ( mSurroundingTetraedreMoveDist <= 0.5f )
                {
                    mReactiveCam.mCamParams.mRotationSpeed = 0.025f;

                    if ( mbeatCounter_3 > 0.0f ) mbeatCounter_3 -= 0.01f;
                    if ( mbeatCounter_3 < 0.0f ) mbeatCounter_3 = 0.0f;
                    
                    if ( mWaveCounter > 0.032f )
                    {
                        mWaveCounter -= 0.005f;
                    }
                    if ( mWaveCounter < 0.032f ) mWeight = 0.032f;
                }
                if ( mSurroundingTetraedreMoveDist == 0.0f )
                {
                    mReactiveCam.mCamParams.mRotationSpeed = 0.015f;
                    if ( mRedCoef > 0.0f ) mRedCoef -= 0.01f;
                    if ( mRedCoef < 0.0f ) mRedCoef = 0.0f;
                    if ( mWeight > 0.1f ) mWeight -= 0.01f;
                    mHdrGamma = clamp( 1.3f + energy1, 0.0f, 2.0f);
                    mDecay = 0.98f;
                    mFreq2 = 0.0f;
                    mFreq = 0.0f;
                    mWeight = 0.01f;
                    mDecay = 0.98f;
                    mDensity = 0.13f + energy2;
                    mExposure = 0.10f + energy1;
                    if ( mWeight > 0.1f ) mWeight -= 0.01f;
                }
            }
        }
    }
    

    
    /*app::console() << "mStep " << mXMLcom.mStep << std::endl;
    app::console() << "mIsBeat0 " << mXMLcom.mIsBeat0 << std::endl;
    app::console() << "mEnergy0 " << mXMLcom.mEnergy0 << std::endl;
    app::console() << "mIsBeat1 " << mXMLcom.mIsBeat1 << std::endl;
    app::console() << "mEnergy1 " << mXMLcom.mEnergy1 << std::endl;
    app::console() << "mIsBeat2 " << mXMLcom.mIsBeat2 << std::endl;
    app::console() << "mEnergy2 " << mXMLcom.mEnergy2 << std::endl;
    app::console() << "mIsRangeBeat " << mXMLcom.mIsRangeBeat << std::endl;
    app::console() << "mRangeEnergy " << mXMLcom.mRangeEnergy << std::endl;
    app::console() << "mReadTime " << mXMLcom.mReadTime << std::endl;*/
}


void hoofeli03App::transition_0( bool start )
{
    if ( start == true )
    {
        mSurroundingTetraedreMove = 1;
    
        if ( mFreq2 < 9.0f ) mFreq2 += 0.004f; // ref 3.98f
        if ( mWaveCoef > 0.2f ) mWaveCoef -= 0.02f; // Beat
    
        // PBR
        mMetallic = 1.00f;
        if ( mRoughness < 0.50f ) mRoughness += 0.1f;
    
        // God Rays
        if ( mExposure > 1.0f ) mExposure -= 0.5f;
        if ( mExposure > 0.31f ) mExposure -= 0.01f;
        mDecay = 0.98f;
        if ( mDensity < 0.54f ) mDensity += 0.01f;
        if ( mWeight < 0.18f ) mWeight += 0.01f;

        // HDR
        mMixBloomGodrays = 1.0f;
        if ( mHdrExposure < 1.93f ) mHdrExposure += 0.01f;
        if ( mHdrGamma > 0.50f ) mHdrGamma -= 0.02f;

        // Drawing
        mMode       = 2;
        if ( mExposure <= 0.31f) mRedCoef = 0.001f;
        mSizeCoef   = 0.00f;
    
        // Ligthdata
        mLightCamera.distanceRadius     = 20.0f;
        if ( mDistanceConverstion > 10.3f ) mDistanceConverstion -= 0.05f;
        if ( mDistanceConverstion < 10.3f ) mDistanceConverstion = 10.3f;

    }
}


void hoofeli03App::transition_1( bool start )
{
    if ( start == true )
    {
        // Freq
        mFreq = 2.5f;
        if ( mFreq2 < 9.0f ) mFreq2 += 0.0015f; // ref 3.98f
        if ( mWaveCoef > 0.3f ) mWaveCoef -= 0.005f; // Beat
        
        // PBR
        mMetallic = 1.00f;
        if ( mRoughness < 0.50f ) mRoughness += 0.1f;
        
        // God Rays
        if ( mExposure < 0.43f ) mExposure += 0.01f;
        mDecay = 0.97f;
        if ( mDensity > 0.51f ) mDensity -= 0.01f;
        if ( mWeight > 0.5f ) mWeight -= 0.01f;
        
        // HDR
        mMixBloomGodrays    = 1.0f;
        if ( mHdrExposure < 1.80f ) mHdrExposure += 0.01f;
        if ( mHdrGamma > 0.5f ) mHdrGamma -= 0.01f;
        
        // Drawing
        mMode = 1;
        if ( mRedCoef < 0.02f) mRedCoef += 0.0001f;
        if ( mSizeCoef < 0.015f ) mSizeCoef += 0.00001f;
        
        // Ligthdata
        mLightCamera.distanceRadius     = 20.0f;
        if ( mDistanceConverstion > 10.3f ) mDistanceConverstion -= 0.1f;
        if ( mDistanceConverstion < 10.3f ) mDistanceConverstion = 10.3f;

    }
}


void hoofeli03App::transition_2( bool start )
{
    if ( start == true )
    {
        // Freq
        mFreq = 3.0f;
        mFreq2 = 6.0f; // ref 3.98f
        if ( mWaveCoef > 0.1f ) mWaveCoef -= 0.1f; // Beat
        
        // PBR
        mMetallic = 1.00f;
        if ( mRoughness < 0.50f ) mRoughness += 0.1f;
        
        // God Rays
        if ( mExposure < 0.43f ) mExposure += 0.01f;
        mDecay = 0.97f;
        if ( mDensity > 0.51f ) mDensity -= 0.01f;
        if ( mWeight > 0.5f ) mWeight -= 0.01f;
        
        // HDR
        mMixBloomGodrays    = 1.0f;
        if ( mHdrExposure < 1.80f ) mHdrExposure += 0.01f;
        if ( mHdrGamma > 0.5f ) mHdrGamma -= 0.01f;
        
        // Drawing
        mMode = 0;
        if ( mRedCoef < 0.06f) mRedCoef += 0.001f;
        mSizeCoef = 0.0f;
        
        // Ligthdata
        mLightCamera.distanceRadius     = 20.0f;
        if ( mDistanceConverstion > 10.3f ) mDistanceConverstion -= 0.1f;
        if ( mDistanceConverstion < 10.3f ) mDistanceConverstion = 10.3f;
    }

}


void hoofeli03App::transition_3( bool start )
{
    if ( start == true )
    {
        // PBR
        mMetallic   = 1.00f;
        mRoughness  = 0.40f;
    
        // HDR
        mMixBloomGodrays    = 1.0f;
    
        mSizeCoef   = 0.0f;
    }
}


void hoofeli03App::mouseDown( MouseEvent event )
{
    mIsBeat = true;
}

void hoofeli03App::mouseDrag( MouseEvent event )
{
}


void hoofeli03App::keyDown( KeyEvent event )
{
}


CINDER_APP( hoofeli03App, RendererGl( RendererGl::Options() ), prepareSettings )
