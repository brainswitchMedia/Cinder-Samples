#pragma once
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace std;
using std::vector;
using namespace cinder;

class DeformedSphere
{
public:
    DeformedSphere();
    gl::                        VboMeshRef create();
    void                        setRadius( float v );
    void                        setCenter( const ci::vec3& v );
    void                        setNumSubdivision( const int subdisivion );
    
    
protected:
    
    void calculateTangents( size_t numIndices, const uint16_t *indices, size_t numVertices, const vec3 *positions, const vec3 *normals, const vec2 *texCoords, vector<vec3> *resultTangents );

    
    cinder::gl::VboMeshRef      mVboMesh;
    std::vector<uint16_t>       mIndices;
    std::vector<ci::vec3>       mPosCoords;
    std::vector<ci::vec3>       mNormals;
    std::vector<ci::vec3>       mTangents;
    std::vector<ci::vec2>       mTexCoords;
    std::vector<vec3>           mColors;
    

    float                       mRadius;
    ci::vec3                    mCenter;
    int                         mSubdivisions;

    int                         mRes;
    float                       mSize;
    
    int                         mNumRings;
    int                         mNumSegments;
    
    float                       mRadiusSizeCoef;
    float                       mRandCoef;
    
};
