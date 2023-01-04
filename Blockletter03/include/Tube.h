#pragma once
#include "cinder/app/App.h"
#include "cinder/BSpline.h"
#include "cinder/Matrix.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"

using namespace cinder;
using namespace std;


class Tube {
public:
    
    Tube();
    Tube( const Tube& obj );
    Tube( const BSpline3f& bspline, const std::vector<vec3>& prof );
    Tube( const std::vector<vec3>& prof, float aScale0, float aScale1, int tubenum );
    ~Tube() {}
    
    Tube& operator = ( const Tube& rhs );
    
    void					setBSpline( const BSpline3f& bspline ) { mBSpline = bspline; }
    void					setProfile( const std::vector<vec3>& prof ) { mProf = prof; }
    void static             setNumSegments( int n ) { mNumSegs = n; }
    
    void					sampleCurve();
    
    // Builds parallel transport frames for each curve sample
    void					buildPTF();
    
    // Builds frenet frames for each curve sample - optional method
    void					buildFrenet();
    
    // Creates a 'tubed' mesh using the profile points
    void					buildMesh( ci::TriMesh *tubeMesh, const int lenght );
    
    void					draw();
    void					drawPs( float lineWidth = 1.0f );
    void					drawTs( float lineLength = 1.0f, float lineWidth = 1.0f );
    void					drawFrames( float lineLength = 1.0f, float lineWidth = 1.0f  );
    void					drawNs( float lineWidth = 1.0f, int lenght = 10 );
    void					drawFrameSlices( float scale = 1.0f );
    
    float					mScale0;		// min scale of profile along curves
    float					mScale1;		// max scale of profile along curves
    
    // VBO TUBE /////////////////////////////////////////////////////////////////////////////////////
    void                    drawTangents( int lenght );
    void static             initVBOMesh( int numSegs );
    void                    buildVBOMesh( int lenght, bool isTubeClosed );
    void                    buildTube( bool isTubeClosed );
    void                    updateVBOMesh( bool isTubeClosed, vec3 origine );
    void                    drawVBOMesh();
    mat3                    rotationAlign( vec3 d, vec3 z );
    void static             computeProfileCorrectionTab();
    
    
    int                                 mTubeNum;
    float                               mRandom;
    
    
protected:
    // VBO TUBE /////////////////////////////////////////////////////////////////////////////////////
    cinder::gl::VboMeshRef              mVboMesh;
    std::vector<ci::vec3>               mPosCoords;
    std::vector<ci::vec3>               mNormals;
    std::vector<ci::vec3>               mTangents;
    static std::vector<uint16_t>        mIndices;
    static std::vector<ci::vec2>        mTexCoords;
    
    
private:
    static int                          mNumSegs;   // Number of segments to sample on the b-spline
    BSpline3f                           mBSpline;   // b-spline path
    std::vector<vec3>                   mProf;      // points to tube (read extrude)
    std::vector<vec3>                   mPs;        // Points in b-spline sample
    std::vector<vec3>                   mTs;        // Tangents in b-spline sample
    std::vector<mat4>                   mFrames;    // Coordinate frame at each b-spline sample
    std::vector<vec3>                   mNs;        // Normals
    
    // VBO TUBE /////////////////////////////////////////////////////////////////////////////////////
    static ci::gl::VboMesh::Layout      mLayout;
    static int                          mLength;
    static bool                         mVBOIsUpdated;
    static bool                         mLastIsTubeClosed;
    vec3                                mFirstVertex  = vec3 (0.0f, 0.0f, 0.0f );    // first vertex calculated
    bool                                mRotateVertex = false;
    std::vector<int>                    mVertexOrder;
    int                                 mLastLength = 0;
    bool                                mTubeIsBuilded = false;
    static std::vector<float>           mProfileEaseOutExpoCorrectionVal;
};


void makeCircleProfile( std::vector<vec3>& prof, float rad = 0.25f, int segments = 100 );
void makeStarProfile( std::vector<vec3>& prof, float rad = 0.25f );
void makeHypotrochoid( std::vector<vec3>& prof, float rad = 0.25f );
void makeEpicycloid( std::vector<vec3>& prof, float rad = 0.25f );
