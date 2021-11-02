#pragma once
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace std;
using std::vector;
using namespace cinder;

struct Triangle
{
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 n0;
    vec3 n1;
    vec3 n2;
    vec3 normal;
    int  indicev0;
    int  indicev1;
    int  indicev2;

};

class CubeSphere
{
public:
    CubeSphere();
    gl::VboMeshRef              create(bool change);
    gl::VboMeshRef              create();

    int                         getNumVertices();
    int                         getNumIndices();
    void                        generateFace( const vec3 &faceCenter, const vec3 &uAxis, const vec3 &vAxis, int subdiv,
                                  vector<vec3> *positions, vector<vec3> *normals, vector<vec3> *tangents,
                                  vector<vec2> *texCoords, vector<uint16_t> *indices );
    ci::vec3                    compute_triangle_normal(struct Triangle *t);
    void                        make_unit_cube_triangles();
    void                        make_unit_cube_triangles( int face, int subdivisions);
    void                        unit_cube(int subdivisions);
    void                        unit_spherified_cube(bool change);
    void                        set_flat_shading_vertex_normals();
    void                        sample_spherical_cubemap_tangent_and_bitangent();
    
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
    
    std::vector<Triangle>       mTriangles;
    std::vector<ci::vec3>       mVertex;

    float                       mRadius;
    ci::vec3                    mCenter;
    int                         mSubdivisions;

    int                         mRes;
    float                       mSize;
    
    int                         numTriangles;
    int                         numVertices;

    

    
    
};
