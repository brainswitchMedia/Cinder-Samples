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


class hoofeli04App : public App {
public:
    hoofeli04App();
    void prepareSettings( App::Settings* settings );
    
    void            createPrimitivesPositionsFromCube();
    gl::VboMeshRef  createPrimitivePyramidFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4, bool cavity );
    void            setSpheresPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices );
    void            setPyramidsPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices );
    
    void            sortIndicesCopy( const std::vector<uint16_t> &indices );
    void            uniqueVertexsCopy( const std::vector<vec3> &vertexPositions );
    vec3            computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 );
    vec3            computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 );
    mat4            rotationAlign( vec3 d, vec3 z );
    void            subdivide( int mSubdivision );
    float           linearize( float a0, float a1, float b0, float b1, float in );

    void setup() override;
    void resize() override;
    void update() override;
    void draw() override;
    void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    
private:
    void loadShaders();
    void loadTextures();
    void saveImage();
    void createGeometry();
    void createParams();
    void setXMLAnimationPhases();
    void transition_0( bool start );
    void transition_1( bool start );
    void transition_2( bool start );
    void transition_3( bool start );
    
    // Tetraedre buffers
    std::vector<uint16_t>       mIndices;
    std::vector<ci::vec3>       mPosCoords;
    std::vector<uint16_t>       mCheckedIndices;
    std::vector<ci::vec3>       mUniquePosCoords;
    
    // Tetraedre vars
    vec3                        mUnitCube[8];
    int                         mSubdivisions = 2;
    
    // Num Spheres
    //static const int            numSpheres = 32; // sub 1
    static const int            numSpheres = 128; // sub 2
    //static const int            numSpheres = 512; // sub 3
    //static const int            numSpheres = 2048; // sub 4
    //static const int            numSpheres = 8192; // sub 5
    //static const int            numSpheres = 32768; // sub 6
    
    // Pyramids buffers
    static const int            numPyramids = numSpheres/2; // sub 4
    gl::BatchRef                mPyramids[ numPyramids ];
    cinder::gl::VboMeshRef      mVboMeshPyramids;
    std::vector<uint16_t>       mIndicesPyramids;
    std::vector<ci::vec3>       mPosCoordsPyramids;
    std::vector<ci::vec3>       mNormalsPyramids;
    std::vector<ci::vec3>       mTangentsPyramids;
    std::vector<ci::vec2>       mTexCoordsPyramids;

    // Spheres buffers
    gl::BatchRef                mSpheres[ numSpheres ];
    std::vector<SphereParticle> mParticlesPositions;
    std::vector<ci::vec3>       mSpheresPositions;
    std::vector<ci::vec3>       mSpheresLights, mSpheresLightsInitialPositions;

    // Instacied particles
    InstanciedParticles         mInstanciedParticles;
    
    // Camera
    vec3                        mCameraTarget = vec3( 0.0, 0.0, 0.0 );
    ReactiveCam                 mReactiveCam;
    bool                        mIsBeat;
    
    // Scene Rotation
    mat4                        mSceneRotation;
    
    // GLSL & Rendering
    gl::BatchRef                mBatchGodRays, mBatchTrailRect, mBatchBloomRect, mBatchToScreenRect, mBatchHdrRect, mBatchFxaaRect;
    
    gl::GlslProgRef             mWireframeShader, mTrailShader, mGodRaysShader, mBloomShader, mHdrShader, mFxaaShader;
    bool                        mEnableFaceFulling;
    
    // FBO
    gl::FboRef                  mRenderFBO, mTrailFBO, mGodRaysFBO, mBloomFBO, mHdrFBO;
    ci::gl::Texture2dRef        mTexturemRender[ 6 ];
    ci::gl::Texture2dRef		mTextureFboTrail[ 4 ];
    ci::gl::Texture2dRef        mTextureGodRays[ 1 ];
    ci::gl::Texture2dRef        mTextureHdr[ 1 ];
    ci::gl::Texture2dRef        mTextureBloom[ 2 ];
    ci::gl::TextureRef          mTextureGradiant;
    
    // FBO Size
    //vec2                        mFboSize = vec2 (3360, 2100);
    vec2                        mFboSize = vec2 (1920 , 1080);
    //vec2                        mFboSize = vec2 (1080 , 1920);
    //vec2                        mFboSize = vec2 (1920, 1920);
    //vec2                        mFboSize = vec2 (3840, 3840);
    //vec2                        mFboSize = vec2 (3840, 2160);
    
    // Drawing Objects
    int                         mTexturingMode;
    float                       mBrightness, mParticlesBrightness;
    float                       mPyramidsIlluminationSize;
    float                       mLightPower;
    float                       mLightPowerParticle;
    float                       mDistancePower;
    
    // Distance shader
    LightData                   mLightCamera;
    float                       mDistanceConverstionParticles;
    float                       mDistanceConverstionTetraedre;
    float                       mParticlesTranslulencyPower;
    float                       mSizeCoef;
    float                       mSizeParticle;
    float                       mSizeParticleMax;
    bool                        mTranslateParticles;
    bool                        mAdjustSize;

    // PBR
    float                       mMetallic;
    float                       mRoughness;
    
    // TRAILS
    size_t                      ping;
    size_t                      pong;
    float                       mTrailAttenuation;

    
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
    float                       mEnergyCoef_1 = 0.2f;
    float                       mHdrGammaCounter = 0.0f;
    
    // SAVE IMAGE ///////////////////////////////////////////////
    int                         mCurrentFrame = 0;
    int                         mFrameCounter = 0;
    bool                        mSaveScreen = false;
    bool                        mLowfpsXML = false;
};


void prepareSettings( App::Settings* settings )
{
    settings->setWindowSize( 1080, 1080 );
    settings->setHighDensityDisplayEnabled();
    settings->setMultiTouchEnabled( false );
    settings->disableFrameRate();
}


hoofeli04App::hoofeli04App()
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


void hoofeli04App::createPrimitivesPositionsFromCube()
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
    
    // subdivide all triangles
    subdivide( mSubdivisions );
    
    size_t mPosCoordszise = mPosCoords.size();
    
    for( uint16_t i = 0; i < mPosCoordszise; ++i )
    {
        console() << " mPosCoord " << mPosCoords[i] << std::endl;
    }
    
    uniqueVertexsCopy( mPosCoords );
    setSpheresPositions( mPosCoords, mIndices );
    setPyramidsPositions( mPosCoords, mIndices );
}


gl::VboMeshRef hoofeli04App::createPrimitivePyramidFromPoints( vec3 point1, vec3 point2, vec3 point3, vec3 point4, bool cavity )
{
    mPosCoordsPyramids.clear();
    mIndicesPyramids.clear();
    mTexCoordsPyramids.clear();
    mNormalsPyramids.clear();
    mTangentsPyramids.clear();
    
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
    
    
    vec3 normal = computeTriangleNormal( point3, point4, point1 );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    
    normal = computeTriangleNormal( point1, point4, point2 );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    
    normal = computeTriangleNormal( point2, point4, point3 );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    mNormalsPyramids.push_back( normal );
    
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


// Create positions for spheres on vertex positions and triangles gravity centers with vertex positions from a tetraedron
void hoofeli04App::setSpheresPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices )
{
    size_t numUniquePosCoord = mUniquePosCoords.size();
    
    // Positions not used here
    for( uint16_t i = 0; i < numUniquePosCoord; ++i )
    {
        //mSpheresPositions.push_back( mUniquePosCoords[i] );
    }
    
    size_t numTriangles = indices.size() / 3;
    
    vec3 cubeCorner0 = mUnitCube[1]; // -> face 0 = ( mUnitCube[2], mUnitCube[0], mUnitCube[5] )
    vec3 cubeCorner1 = mUnitCube[3]; // -> face 1 = ( mUnitCube[7], mUnitCube[0], mUnitCube[2] )
    vec3 cubeCorner2 = mUnitCube[4]; // -> face 2 = ( mUnitCube[0], mUnitCube[7], mUnitCube[5] )
    vec3 cubeCorner3 = mUnitCube[6]; // -> face 3 = ( mUnitCube[7], mUnitCube[2], mUnitCube[5] )
    
    for( uint16_t i = 0; i < numTriangles; ++i ) {
        uint16_t index0 = indices[i * 3 + 0];
        uint16_t index1 = indices[i * 3 + 1];
        uint16_t index2 = indices[i * 3 + 2];
        
        vec3 faceNormal = computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] );

        vec3 triangleGravityCenter = computeTriangleGravitycenter( posCoords[index0], posCoords[index1], posCoords[index2] );
        mSpheresPositions.push_back( triangleGravityCenter - computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] ) * 0.025f );
        
        vec3 position = triangleGravityCenter - computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] ) * 0.025f;

        float lenght0 = length( position - cubeCorner0 );
        float lenght1 = length( position - cubeCorner1 );
        float lenght2 = length( position - cubeCorner2 );
        float lenght3 = length( position - cubeCorner3 );
        
        float distToFaceGravityCenter = 0.0f;

        if( lenght0 - lenght1 < 0.0f && lenght0 - lenght2 < 0.0f && lenght0 - lenght3 < 0.0f )
        {
            distToFaceGravityCenter = length( position - computeTriangleGravitycenter( mUnitCube[2], mUnitCube[0], mUnitCube[5] ) );
        }
        else if ( lenght1 - lenght0 < 0.0f && lenght1 - lenght2 < 0.0f && lenght1 - lenght3 < 0.0f )
        {
            distToFaceGravityCenter = length( position - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[0], mUnitCube[2] ) );
        }
        else if ( lenght2 - lenght0 < 0.0f && lenght2 - lenght1 < 0.0f && lenght2 - lenght3 < 0.0f )
        {
            distToFaceGravityCenter = length( position - computeTriangleGravitycenter( mUnitCube[0], mUnitCube[7], mUnitCube[5] ) );
        }
        else if ( lenght3 - lenght0 < 0.0f && lenght3 - lenght1 < 0.0f && lenght3 - lenght2 < 0.0f )
        {
            distToFaceGravityCenter = length( position - computeTriangleGravitycenter( mUnitCube[7], mUnitCube[2], mUnitCube[5] ) );
        }
        console() << "distToFaceGravityCenter: " << distToFaceGravityCenter << std::endl;

        
        // Sphere radius = 0.8
        //mSpheresLights.push_back( triangleGravityCenter - computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] ) * 0.9f );
        // Sphere radius = 0.2
        mSpheresLights.push_back( triangleGravityCenter - computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] ) * 1.0f );
        
        vec3 ligthPositionSphere = triangleGravityCenter - computeTriangleNormal( posCoords[index0], posCoords[index1], posCoords[index2] ) * 0.9f ;
        
        //console() << " num mSpheres Positions " << mSpheresPositions.size() << std::endl;
        // Max gravityCneter value is 1.225
        // -> convert to 0 - 1
        float distToFaceGravityCenterLin = linearize( 0.0f, 1.225f , 1.0f, 0.5f, distToFaceGravityCenter );
        mParticlesPositions.push_back( SphereParticle( position, ligthPositionSphere, faceNormal, distToFaceGravityCenterLin ) );
    }
    

    
    mSpheresLightsInitialPositions = mSpheresLights;
}


// Create small tetraetdrons on all faces off a bigger tetraedron
void hoofeli04App::setPyramidsPositions( const std::vector<ci::vec3> &posCoords, const std::vector<uint16_t> &indices )
{
    size_t numTriangles = indices.size() / 3;
    
    for( uint16_t i = 0; i < numTriangles; ++i ) {
        uint16_t index0 = indices[i * 3 + 0];
        uint16_t index1 = indices[i * 3 + 1];
        uint16_t index2 = indices[i * 3 + 2];
        
        const vec3 &v0 = mPosCoords[index0];
        const vec3 &v1 = mPosCoords[index1];
        const vec3 &v2 = mPosCoords[index2];
        
        vec3 triangleGravityCenter = computeTriangleGravitycenter( posCoords[index0], posCoords[index1], posCoords[index2] );
        
        // edge length: AB=sqrt((xB−xA)2+(yB−yA)2)
        float edgeLength = sqrtf( pow( posCoords[index0].x - posCoords[index1].x, 2 ) + pow( posCoords[index0].y - posCoords[index1].y, 2 ) + pow( posCoords[index0].z - posCoords[index1].z, 2 ));
        
        //heigth = √(2/3) × edgelength
        float pyramidHeigth = sqrt( 2.0f / 3.0f ) * edgeLength;
        
        bool cavity = true;
        float PyramidHeigth = 1.0f;
        if ( cavity == true ) PyramidHeigth = -0.2f;
        
        mPyramidsPositions.push_back( triangleGravityCenter );
        mPyramids[i] = gl::Batch::create( createPrimitivePyramidFromPoints( posCoords[index0], posCoords[index1], posCoords[index2], triangleGravityCenter + PyramidHeigth * computeTriangleNormal( v0, v1, v2 ) * pyramidHeigth, cavity ), mWireframeShader );
        //mPyramids[i] = gl::Batch::create( createPrimitivePyramidFromPoints( posCoords[index0], posCoords[index1], posCoords[index2], triangleGravityCenter + computeTriangleNormal( v0, v1, v2 ) * 0.02f ), mWireframeShader );
    }
}


vec3 hoofeli04App::computeTriangleGravitycenter( const vec3 &point1, const vec3 &point2, const vec3 &point3 )
{
    return vec3( point1.x + point2.x + point3.x, point1.y + point2.y + point3.y, point1.z + point2.z + point3.z ) / 3.0f;
}


// Rotate an object fron angle d and vector z => Inigo Quilez
mat4 hoofeli04App::rotationAlign( vec3 d, vec3 z )
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


vec3 hoofeli04App::computeTriangleNormal( const vec3 &pos1, const vec3 &pos2, const vec3 &pos3 )
{
    vec3 e0 = pos2 - pos1;
    vec3 e1 = pos3 - pos1;
    
    return normalize( cross( e0, e1 ));
}


// Sort and remove duplicates indices
void hoofeli04App::sortIndicesCopy( const std::vector<uint16_t> &indices )
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
void hoofeli04App::uniqueVertexsCopy( const std::vector<vec3> &vertexPositions )
{
    for ( std::vector<vec3>::const_iterator it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
    {
        if ( std::find( mUniquePosCoords.begin(), mUniquePosCoords.end(), *it ) == mUniquePosCoords.end() )
        {
            mUniquePosCoords.push_back( *it );
        }
    }
}


void hoofeli04App::subdivide( int mSubdivision )
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


// conv lin from [a0-a1] to [b0-b1]
// a0-a1 -> b0-b1 - ex: 0.0-2.0 -> 0.0-1.0
// out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
float hoofeli04App::linearize( float a0, float a1, float b0, float b1, float in )
{
    return ( ( in - a0 ) / ( a1 - a0 ) ) * ( b1 - b0 ) + b0;
}

void hoofeli04App::setup()
{
    //gl::enableVerticalSync( true );
    
    // Setup animation variables
    mCounter_1  = 0.0f;
    
    // PBR
    mMetallic   = 1.00f;
    mRoughness  = 0.40f;
    
    // God Rays
    mExposure   = 0.1f;
    mDecay      = 0.98f;
    mDensity    = 0.13f;
    mWeight     = 0.01f;
    
    // Trails
    ping = 0;
    pong = 1;
    mTrailAttenuation = 0.670f;
    
    // Bloom
    aBloomValues[0] = 0.227027f;
    aBloomValues[1] = 0.1945946f;
    aBloomValues[2] = 0.1216216f;
    aBloomValues[3] = 0.054054f;
    aBloomValues[4] = 0.016216f;
    
    // HDR
    mMixBloomGodrays        = 0.0f;
    mHdrExposure            = 1.9f;
    mHdrGamma               = 1.3f;
    mChromaticAberration    = 0.0013f;
    
    // Drawing Objects
    mSizeCoef = 0.0f;
    mSizeParticle = 0.0f;
    mSizeParticleMax = 0.0f;
    mTranslateParticles = false;
    mAdjustSize = false;
    mTexturingMode = 0;
    mBrightness = 0.059f;
    mParticlesBrightness = 0.57f;
    mPyramidsIlluminationSize = 1.017f;
    mLightPower = 50.0f;
    mLightPowerParticle = 6.6f;
    mDistancePower = 0.53f;
    
    // Ligthdata
    mLightCamera.distanceRadius     = 20.0f;
    mLightCamera.viewpoint          = vec3( mLightCamera.distanceRadius );
    mLightCamera.fov                = 60.0f;
    mLightCamera.target             = vec3( 0 );
    mLightCamera.toggleViewpoint	= false;
    mDistanceConverstionParticles   = 10.3f;
    mDistanceConverstionTetraedre   = 10.3f;
    mParticlesTranslulencyPower     = 8.0f;
    
    // ReactiveCam
    mReactiveCam.set( 6.0f, mCameraTarget, 0.01f, 10.0f );
    
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
    
    // Setup objects position
    createPrimitivesPositionsFromCube();
    
    // Create the meshes.
    createGeometry();
    
    // Init instancied particles
    mInstanciedParticles.setup( &( mReactiveCam.getCamera()), &mLightCamera.camera, mParticlesPositions, 0 );
    
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


void hoofeli04App::update()
{
    vec3 rotationVector = vec3( 0.25f, -1.5f, 0.5f ) ;

    // Scene rotation
    mSceneRotation *= rotate( 0.005f, rotationVector  );
    
    // Rotate Pyramids lights
    int numPyramidsLights = mSpheresLights.size();
    
    for( int p = 0; p < numPyramidsLights; ++p )
    {
        mSpheresLights[p] = mat3(mSceneRotation) * mSpheresLightsInitialPositions[p];
    }
    
    // Update Camera
    mReactiveCam.update( mIsBeat, normalize( vec3( 0.5f, -1.5f, 0.5f ) ), true, 0.0f );
    mLightCamera.viewpoint = mLightCamera.distanceRadius * normalize( mReactiveCam.getPosition() ) * -1.0f;
    mLightCamera.camera.lookAt( mLightCamera.viewpoint, mLightCamera.target );
    mIsBeat = false;
    
    // Update instanced particles
    mInstanciedParticles.update( mSizeCoef, mSceneRotation, mFboSize, mReactiveCam.getPosition(), mTranslateParticles, mAdjustSize );
    
    // COM
    if ( mPlayXML == TRUE  && mCurrentFrame > 368 )
    {
        mXMLcom.update();
        setXMLAnimationPhases();
    }
    
    // Transitions
    transition_0( mStart_Transition0 );
    transition_1( mStart_Transition1 );
    transition_2( mStart_Transition2 );
    transition_3( mStart_Transition3 );
    
    // Counters
    // 60 fps
    mCounter_1 += 0.01667f;
}


void hoofeli04App::draw()
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
        
        // Render Particles
        {
            // Render Frontside only
            gl::ScopedFaceCulling cullScope( mEnableFaceFulling, GL_BACK );
            gl::cullFace( GL_BACK );
            mInstanciedParticles.drawInstanced( mDistanceConverstionParticles, mParticlesBrightness, mLightPowerParticle, mParticlesTranslulencyPower, mTexturingMode, mCounter_1 );
        }

        /*{
   
            
            int numSpheresToDraw = mSpheresPositions.size();
            //console() << " numSpheresToDraw " << numSpheresToDraw << std::endl;
            
            for( int s = 0; s < numSpheresToDraw; ++s )
            {
                gl::ScopedTextureBind scopedTextureBind( mTextureStripes );
                mPhongShader->uniform( "uTexturingMode", mTexturingMode );
                mPhongShader->uniform( "uBrightness", mParticlesBrightness );
                mPhongShader->uniform( "uLightPosition", mSpheresLights[s] );
                gl::pushModelMatrix();
                gl::translate( mSpheresPositions[s] );
                mSpheres[s]->draw();
                gl::popModelMatrix();
            }
        }*/
        
        {
            gl::ScopedTextureBind scopedTextureBind( mTextureGradiant );
            mWireframeShader->uniform( "uBrightness", 0.11f * mBrightness );
            mWireframeShader->uniform( "uLineStyle", 1 );
            mWireframeShader->uniform( "uTexGradiant", 0 );
            mWireframeShader->uniform( "uColor", vec3( 0.5f, 0.5f, 0.5f ) );// vert
            mWireframeShader->uniform( "uSize", mPyramidsIlluminationSize );
            mWireframeShader->uniform( "uPower1", mDistancePower );
            mWireframeShader->uniform( "uPower2", mLightPower );
            mWireframeShader->uniform( "uWorldtoLightMatrix", mLightCamera.camera.getViewMatrix() );
        
            // Render the back side first.
            gl::ScopedBlendAlpha blendScope;
            gl::multModelMatrix( mSceneRotation );

            gl::ScopedFaceCulling cullScope( true, GL_BACK );
            for( int p = 0; p < numPyramids; ++p )
            {
                mWireframeShader->uniform( "uLightPosition", mSpheresLights[p] );
                mWireframeShader->uniform( "uLightModelView", mReactiveCam.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
                // For the back we need less attenuation
                mWireframeShader->uniform( "uDistanceConverstion", 0.0f );
                mPyramids[p]->draw();
            }
        
            gl::cullFace( GL_FRONT );
            for( int p = 0; p < numPyramids; ++p )
            {
                mWireframeShader->uniform( "uLightPosition", mSpheresLights[p] );
                mWireframeShader->uniform( "uLightModelView", mReactiveCam.getViewMatrix() * vec4( 0.0, 0.0, 0.0, 1.0 ) );
                mWireframeShader->uniform( "uDistanceConverstion", mDistanceConverstionTetraedre );
                mPyramids[p]->draw();
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
             mBatchBloomRect->getGlslProg()->uniform( "uWidth", height );
             mBatchBloomRect->getGlslProg()->uniform( "uHeight", width );
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
            const gl::ScopedTextureBind scopedTextureBind1( mTextureBloom[(int)!horizontal], 1 );
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
        const gl::ScopedTextureBind scopedTextureBind1( mTextureFboTrail[ ping ], 0 ); // bloom texture
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
        //const gl::ScopedTextureBind scopedTextureBind0( mTextureFboTrail[ ping ], 0 ); // trail texture
        const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 4 ], 0 );
        //const gl::ScopedTextureBind scopedTextureBind0( mTexturemRender[ 2 ], 0 );
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


void hoofeli04App::resize()
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


void hoofeli04App::createGeometry()
{
    int numSpheresToDraw = mSpheresPositions.size();
    
    for( int i = 0; i < numSpheresToDraw; ++i ) {
        auto sphere = geom::Sphere().radius( 0.025f ).subdivisions( 20 );
        mSpheres[i] = gl::Batch::create( sphere, mPhongShader );
    }
    
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


void hoofeli04App::loadTextures()
{
    // Load the textures.
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    mTextureGradiant = gl::Texture::create( loadImage( loadAsset( "textures/gradiant0.jpg" ) ), fmt );
}


// Save renderFbo
void hoofeli04App::saveImage()
{
    //for 30 fps
    if (  ( mLowfpsXML == true && mCurrentFrame % 2 == 0 && mXMLcom.isLooping() == true ) || ( mCurrentFrame == 368 ) )
    {
        //console() << "mCurrentFrame saved : " << mCurrentFrame << std::endl;
        //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mFrameCounter ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
        mFrameCounter++;
    }
    else if ( ( mXMLcom.isLooping() == true ) || ( mCurrentFrame == 368 ) )
    {
        // Pull down the current window as a surface and pass it to writeImage
        //writeImage( getHomeDirectory() / "CinderScreengrabs" / ( toString(1) + "_" + toString( mCurrentFrame ) + ".png" ), mRenderFBO->getTexture2d( GL_COLOR_ATTACHMENT4 )->createSource(), ImageTarget::Options().quality(1.0f) );
    }
}


void hoofeli04App::loadShaders()
{
    try {
        mTrailShader = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "color.frag" ) );
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
}


void hoofeli04App::createParams()
{
    mParams = params::InterfaceGl::create( getWindow(), "Geometry Demo", toPixels( ivec2( 300, 400 ) ) );
    mParams->setOptions( "", "valueswidth=100 refresh=0.1" );
    mParams->addSeparator();
    mParams->addParam( "Face Culling", &mEnableFaceFulling ).updateFn( [this] { gl::enableFaceCulling( mEnableFaceFulling ); } );
    mParams->addSeparator();
    mParams->addParam( "Lights SizeCoef", &mSizeCoef, "min=-0.0 max=10.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Texturing Mode", &mTexturingMode, "min=-0 max=2 step=1 keyIncr=q keyDecr=w" );
    mParams->addParam( "Pyramid Illum Size", &mPyramidsIlluminationSize, "min=-100.0 max= 100.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Brightness ", &mBrightness, "min=-1000.0 max= 1000.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Light Power Particles ", &mLightPowerParticle, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Light Power ", &mLightPower, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Distance Power ", &mDistancePower, "min=-50.0 max= 50.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Particles Brightness", &mParticlesBrightness, "min=-1000.0 max= 1000.0 step=0.001 keyIncr=q keyDecr=w" );
    mParams->addParam( "Attenuation trail", &mTrailAttenuation, "min=0.0 max=5.0 step=0.001 keyIncr=q keyDecr=w" );

    mParams->addSeparator();
    mParams->addParam( "distance ligth", &mLightCamera.distanceRadius ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "distance converstion Tet", &mDistanceConverstionTetraedre ).min( 0.0f ).step( 0.01f);
    mParams->addParam( "distance converstion Part", &mDistanceConverstionParticles ).min( 0.0f ).step( 0.01f);

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
    mParams->addParam( "Axis X Rot speed", &mReactiveCam.mCamParams.mAxisXRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Axis Y Rot speed", &mReactiveCam.mCamParams.mAxisYRotationSpeed, "min=-5.0 max=5.0 step=0.01 keyIncr=q keyDecr=w" );
    mParams->addParam( "Rotation speed", &mReactiveCam.mCamParams.mRotationSpeed, "min=-1.0 max=1.0 step=0.001 keyIncr=q keyDecr=w" );
}


void hoofeli04App::setXMLAnimationPhases()
{
  if ( mXMLcom.mReadTime < 3.808073 )
  {
      if ( mStart_Transition0 == false ) mStart_Transition0 = true;
      
      float brightness_Min = 0.005f;
      float brightness_Max = 0.3f;
      float lin = linearize( 0.0f, 1.0f, brightness_Min, brightness_Max, mXMLcom.mRangeEnergy );
      mBrightness = lin;
  }
  else if ( mXMLcom.mReadTime >= 3.808073  && mXMLcom.mReadTime < 8.544943 )
  {
      mStart_Transition0 = false;
      if ( mStart_Transition1 == false ) mStart_Transition1 = true;
      
      float brightness_Min = 0.01f;
      float brightness_Max = 0.3f;
      float lin1 = linearize( 0.0f, 1.0f, brightness_Min, brightness_Max, mXMLcom.mRangeEnergy );
      mBrightness = lin1;
      
      float sizeCoef_Min = mSizeParticle;
      float sizeCoef_Max = 3.0f;
      float lin2 = linearize( 0.0f, 1.0f, sizeCoef_Min, sizeCoef_Max, mXMLcom.mEnergy1 );
      mSizeCoef = lin2;
  }
  else if ( mXMLcom.mReadTime >= 8.544943  && mXMLcom.mReadTime < 27.945215 )
  {
      mStart_Transition1 = false;
      if ( mStart_Transition2 == false )
      {
          mStart_Transition2 = true;
      }
      
      float brightness_Min = 0.01f;
      float brightness_Max = 0.3f;
      float lin1 = linearize( 0.0f, 1.0f, brightness_Min, brightness_Max, mXMLcom.mRangeEnergy );
      mBrightness = lin1;
      
      mSizeCoef = mSizeParticle;
      
      float particlesBrightness_Min = 0.2f;
      float particlesBrightness_Max = 1.5f;
      float lin3 = linearize( 0.0f, 1.0f, particlesBrightness_Min, particlesBrightness_Max, pow(mXMLcom.mEnergy1, 0.8f ) );
      mParticlesBrightness = lin3;
  }
  else if ( mXMLcom.mReadTime >= 27.945215 )
  {
      mStart_Transition2 = false;
      if ( mStart_Transition3 == false )
      {
          mStart_Transition3 = true;
      }
      
      if( mXMLcom.mIsRangeBeat == true )
      {
         // mIsBeat = true;
      }
      
      if( mXMLcom.mIsBeat0 == true )
      {
          mbeatCounter_1 = 0.0f;
          mIsBeat = true;
      }
      
      float ease = 1.0 - easeOutExpo( clamp( mbeatCounter_1, 0.0f, 1.0f ) );
      if ( mbeatCounter_1 < 1.0f ) mbeatCounter_1 += 0.015f;

      float sizeCoef_Min = mSizeParticle;
      float sizeCoef_Max = mSizeParticleMax;
      float lin1 = linearize( 0.0f, 1.0f, sizeCoef_Min, sizeCoef_Max, ease );
      mSizeCoef = lin1;
      
      float particlesBrightness_Min = 0.3f;
      float particlesBrightness_Max = 0.6f;
      float lin2 = linearize( 0.0f, 1.0f, particlesBrightness_Min, particlesBrightness_Max, ease );
      mParticlesBrightness = lin2;

  }
}


void hoofeli04App::transition_0( bool start )
{
    if ( start == true )
    {
        mSizeCoef = 0;
        mTexturingMode = 0;
        mParticlesBrightness = 0.0f;
        mLightPowerParticle = 6.6f;
        mDistanceConverstionParticles = 10.3f;
        mParticlesTranslulencyPower = 8.0f;
        mTrailAttenuation = 0.57f;
        mPyramidsIlluminationSize = 1.017f;
        mLightPower = 50.0f;
        mDistancePower = 0.07f;
        mDistanceConverstionTetraedre = 10.3f;
        mChromaticAberration = 0.0018f;
    }
}

void hoofeli04App::transition_1( bool start )
{
    if ( start == true )
    {
        mTranslateParticles = true;
        if ( mSizeParticle < 0.49f ) mSizeParticle += 0.01f;
        mParticlesBrightness = 0.0f;
        mLightPowerParticle = 6.6f;
        mDistanceConverstionParticles = 10.4f;
        mParticlesTranslulencyPower = 8.0f;
        mTrailAttenuation = 0.67f;
        mPyramidsIlluminationSize = 1.017f;
        mLightPower = 50.0f;
        mDistancePower = 0.07f;
        mDistanceConverstionTetraedre = 10.3f;
        mChromaticAberration = 0.0018f;
    }
}

void hoofeli04App::transition_2( bool start )
{
    if ( start == true )
    {
        mAdjustSize = true;
        mTranslateParticles = true;
        mTexturingMode = 0;
        if ( mSizeParticle < 3.0f ) mSizeParticle += 0.004f;
        if ( mParticlesBrightness < 0.049f ) mParticlesBrightness += 0.001f;
        mLightPowerParticle = 6.6f;
        mDistanceConverstionParticles = 10.4f;
        mParticlesTranslulencyPower = 8.0f;
        mTrailAttenuation = 0.47f;
        mPyramidsIlluminationSize = 1.017f;
        mLightPower = 50.0f;
        if ( mDistancePower < 0.20f ) mDistancePower += 0.01f;
        if ( mDistancePower > 0.20f ) mDistancePower = 0.20f;
        mDistanceConverstionTetraedre = 10.3f;
        mChromaticAberration = 0.0018f;
        mSizeParticleMax = mSizeCoef;
    }
}

void hoofeli04App::transition_3( bool start )
{
    if ( start == true )
    {
        mAdjustSize = true;
        mTranslateParticles = true;
        mTexturingMode = 1;
        if ( mSizeParticle < 2.19f ) mSizeParticle += 0.01f;
        if ( mSizeParticleMax < 9.0f ) mSizeParticleMax += 0.05f;
        //if ( mParticlesBrightness < 1.49f ) mParticlesBrightness += 0.01f;
        mLightPowerParticle = 6.6f;
        if ( mDistanceConverstionParticles < 10.55f ) mDistanceConverstionParticles += 0.01f;
        if ( mDistanceConverstionParticles > 10.55f ) mDistanceConverstionParticles = 10.55f;
        mParticlesTranslulencyPower = 12.0f;
        if ( mTrailAttenuation < 0.75f ) mTrailAttenuation += 0.01f;
        if ( mTrailAttenuation > 0.75f ) mTrailAttenuation = 0.75f;
        if ( mBrightness < 1.565f ) mBrightness += 0.01f;
        if ( mBrightness > 1.565f ) mBrightness = 1.565f;
        if ( mPyramidsIlluminationSize < 1.126f ) mPyramidsIlluminationSize += 0.01f;
        if ( mPyramidsIlluminationSize > 1.126f ) mPyramidsIlluminationSize = 1.126f;
        if ( mLightPower > 30.87f ) mLightPower -= 0.05f;
        if ( mLightPower < 30.87f ) mLightPower = 30.87f;
        if ( mDistancePower > -0.99f ) mDistancePower -= 0.002f;
        if ( mDistancePower < -0.99f ) mDistancePower = -0.99f;
        if ( mDistanceConverstionTetraedre < 10.4f ) mDistanceConverstionTetraedre += 0.01f;
        if ( mDistanceConverstionTetraedre > 10.4f ) mDistanceConverstionTetraedre = 10.4f;
        mDistanceConverstionTetraedre = 10.3f;

        if ( mChromaticAberration < 0.0018f ) mChromaticAberration += 0.00001f;
        if ( mChromaticAberration > 0.0018f ) mChromaticAberration = 0.0018f;
    }
}


void hoofeli04App::mouseDown( MouseEvent event )
{
    mIsBeat = true;
}

void hoofeli04App::mouseDrag( MouseEvent event )
{
}


void hoofeli04App::keyDown( KeyEvent event )
{
}


CINDER_APP( hoofeli04App, RendererGl( RendererGl::Options().msaa( 32 ) ), prepareSettings )
