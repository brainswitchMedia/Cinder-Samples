#include "CubeSphere.h"

using namespace ci;
using namespace std;
using namespace cinder;

CubeSphere::CubeSphere(): mRadius( 2.0f ), mCenter(vec3( 0, 0, 0 )), mSubdivisions(95)
{
    cinder::app::console() <<  "jgiu";
}


gl::VboMeshRef CubeSphere::create( bool change )
{
    unit_cube(mSubdivisions);
    unit_spherified_cube( change );
    
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    layout.attrib( geom::TANGENT, 3 );
    layout.attrib( geom::TEX_COORD_0, 2 );//.usage( GL_STATIC_DRAW );
    
    mPosCoords = mVertex;

    calculateTangents( mIndices.size(), mIndices.data(), mPosCoords.size(), mPosCoords.data(), mNormals.data(), mTexCoords.data(), &mTangents );
    
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    mVboMesh->bufferAttrib( geom::TANGENT, mTangents.size() * sizeof( vec3 ), mTangents.data() );
    mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );

    
    return mVboMesh;
}


gl::VboMeshRef CubeSphere::create()
{
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    layout.attrib( geom::TANGENT, 3 );
    layout.attrib( geom::TEX_COORD_0, 2 );//.usage( GL_STATIC_DRAW );
    
    vector<vec3> mPosCoords;
    vector<uint16_t> mIndices;
    vector<vec3> mNormals;
    vector<vec3> mTangents;
    vector<vec2> mTexCoords;
    vector<vec3> *normalsPtr = nullptr;
    vector<vec3> *tangentsPtr = nullptr;
    vector<vec2> *texCoordsPtr = nullptr;

    const size_t numVertices = getNumVertices();
    
    // reserve room in vectors and set pointers to non-null for normals, texcoords and colors as appropriate
    mPosCoords.reserve( numVertices );
    mIndices.reserve( getNumIndices() );
    
    mNormals.reserve( numVertices );
    normalsPtr = &mNormals;

    mTangents.reserve( numVertices );
    tangentsPtr = &mTangents;
    
    mTexCoords.reserve( numVertices );
    texCoordsPtr = &mTexCoords;
    
    
    vec3 sz = vec3(1.0f);
    
    // +X
    generateFace( vec3(1.0,0,0), vec3(0,0,sz.z), vec3(0,sz.y,0), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );
    // +Y
    generateFace( vec3(0,1.0,0), vec3(sz.x,0,0), vec3(0,0,sz.z), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );
    // +Z
    generateFace( vec3(0,0,1.0), vec3(0,sz.y,0), vec3(sz.x,0,0), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );
    // -X
    generateFace( vec3(-1.0,0,0), vec3(0,sz.y,0), vec3(0,0,sz.z), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );
    // -Y
    generateFace( vec3(0,-1.0,0), vec3(0,0,sz.z), vec3(sz.x,0,0), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );
    // -Z
    generateFace( vec3(0,0,-1.0), vec3(sz.x,0,0), vec3(0,sz.y,0), mSubdivisions, &mPosCoords,
                 normalsPtr, tangentsPtr, texCoordsPtr, &mIndices );

    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    mVboMesh->bufferAttrib( geom::TANGENT, mTangents.size() * sizeof( vec3 ), mTangents.data() );
    mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
    
    
    return mVboMesh;
}



void CubeSphere::setRadius( float v )
{
    mRadius = v;
}


void CubeSphere::setCenter( const ci::vec3& v )
{
    mCenter = v;
}


void CubeSphere::setNumSubdivision( const int v )
{
    mSubdivisions = v;
}


int CubeSphere::getNumVertices()
{
    return 2 * ( (mSubdivisions + 1) * (mSubdivisions + 1) ) // +-Z
    + 2 * ( (mSubdivisions + 1) * (mSubdivisions + 1) ) // +-X
    + 2 * ( (mSubdivisions + 1) * (mSubdivisions + 1) ); // +-Y
}


int CubeSphere::getNumIndices()
{
    return 2 * 6 * ( mSubdivisions * mSubdivisions ) // +-Z
    + 2 * 6 * ( mSubdivisions * mSubdivisions ) // +-X
    + 2 * 6 * ( mSubdivisions * mSubdivisions ); // +-Y
}



vec3 CubeSphere::compute_triangle_normal(struct Triangle *t)
{
    vec3 v1, v2, crossp;
    
    v1.x = t->v1.x - t->v0.x;
    v1.y = t->v1.y - t->v0.y;
    v1.z = t->v1.z - t->v0.z;
    
    v2.x = t->v2.x - t->v1.x;
    v2.y = t->v2.y - t->v1.y;
    v2.z = t->v2.z - t->v1.z;
    
    crossp = cross(v1, v2);
    
    /* make sure we always have a valid normal, not NaNs */
    if (length(crossp) < 1e-20) return vec3(0.0f, 1.0f, 0.0f);

    else return normalize(crossp);
}


void CubeSphere::set_flat_shading_vertex_normals()
{
    for (uint i = 0; i < numTriangles; i++) {
        vec3 normal;
        normal = compute_triangle_normal(&mTriangles[i]);
        mTriangles[i].normal.x = normal.x;
        mTriangles[i].normal.y = normal.y;
        mTriangles[i].normal.z = normal.z;
        
        mTriangles[i].n0 = normal;
        mTriangles[i].n1 = normal;
        mTriangles[i].n2 = normal;
    }
}


/*float CubeSphere::mesh_compute_radius()
{
    int i;
    float r, max_radius = 0.0;
    
    for (i = 0; i < numVertices; i++) {
        r = length(mVertex[i]);
        if (r > max_radius)
            max_radius = r;
    }
    return max_radius;
}*/


void CubeSphere::make_unit_cube_triangles( int face, int subdivisions)
{
    int i, j, v1, v2, v3, vindex, tindex;
    
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    tindex = face * (subdivisions * subdivisions) * 2;
    
    for (i = 0; i < subdivisions; i++) {
        for (j = 0; j < subdivisions; j++) {
            v1 = vindex + i + j * (subdivisions + 1);
            v2 = v1 + 1;
            v3 = v1 + subdivisions + 1;
            
            mIndices.push_back( v1 );
            mIndices.push_back( v2 );
            mIndices.push_back( v3 );
            
            mTriangles[tindex].v0 = mVertex[v1];
            mTriangles[tindex].v1 = mVertex[v2];
            mTriangles[tindex].v2 = mVertex[v3];
            
            tindex++;
            
            v1 = v3;
            /* v2 is the same */
            v3 = v1 + 1;
            
            mIndices.push_back( v1 );
            mIndices.push_back( v2 );
            mIndices.push_back( v3 );
            
            mTriangles[tindex].v0 = mVertex[v1];
            mTriangles[tindex].v1 = mVertex[v2];
            mTriangles[tindex].v2 = mVertex[v3];
            
            tindex++;
        }
    }
}


void CubeSphere::generateFace( const vec3 &faceCenter, const vec3 &uAxis, const vec3 &vAxis, int subdiv,
                  vector<vec3> *positions, vector<vec3> *normals, vector<vec3> *tangents,
                  vector<vec2> *texCoords, vector<uint16_t> *indices )
{
    
    const uint32_t baseIdx = (uint32_t)positions->size();
    
    // fill vertex data
    for( int vi = 0; vi <= subdiv; vi++ ) {
        const float v = vi / float(subdiv);
        for( int ui = 0; ui <= subdiv; ui++ ) {
            const float u = ui / float(subdiv);
            
            float x = 1.0f - 2.0f * u;
            float y = 1.0f - 2.0f * v;
            
            float k = sqrt(x*x + y*y + 1);
            
            vec3 position = (faceCenter + x * uAxis + y * vAxis )/k;
            positions->emplace_back( position );
            
            vec3 normal = normalize( position );

            normals->emplace_back( normal );
            
            vec3 tangent = vec3( 1.0f + y*y, -x*y, -x );
 
            tangents->emplace_back( tangent );

            texCoords->emplace_back( u, v );
        }
    }
    
    // 'baseIdx' will correspond to the index of the first vertex we created in this call to generateFace()
    //	const uint32_t baseIdx = indices->empty() ? 0 : ( indices->back() + 1 );
    for( int u = 0; u < subdiv; u++ ) {
        for( int v = 0; v < subdiv; v++ ) {
            const int i = u + v * ( subdiv + 1 );
            
            indices->push_back( baseIdx + i );
            indices->push_back( baseIdx + i + subdiv + 1 );
            indices->push_back( baseIdx + i + 1 );
            
            indices->push_back( baseIdx + i + 1 );
            indices->push_back( baseIdx + i + subdiv + 1 );
            indices->push_back( baseIdx + i + subdiv + 2 );
            // important the last is the highest idx due to determination of next face's baseIdx
        }
    }
}


void CubeSphere::unit_cube(int subdivisions)
{
    int i, j, face, vindex;
    
    numVertices = 6 * (subdivisions + 1) * (subdivisions + 1);
    numTriangles = 6 * (subdivisions * subdivisions) * 2;
    
    mTriangles.assign( numTriangles, Triangle() );
    mVertex.reserve( numVertices );
    mIndices.reserve( numTriangles );
    mNormals.reserve( numVertices );
    mTexCoords.reserve( numVertices );
    
    face = 0; /* normal is positive z */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = (float) i * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mVertex[vindex].y = (float) j * (1.0f / (float) subdivisions) * 2.0f - 1.0f;
            mVertex[vindex].z = 1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(0.0f, 0.0f, 1.0f));
            mTexCoords.push_back( vec2(u,v) );
            vindex++;
        }
    }
    
    
    face = 1; /* normal is positive x */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = 1.0f;
            mVertex[vindex].y = (float) j * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mVertex[vindex].z = (float) i * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(1.0f, 0.0f, 0.0f));
            mTexCoords.push_back( vec2(u,v) );

            vindex++;
        }
    }
    
    face = 2; /* normal is negative z */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = (float) i * (1.0f / (float) subdivisions) * 2.0f - 1.0f;
            mVertex[vindex].y = (float) j * (1.0f / (float) subdivisions) * 2.0f - 1.0f;
            mVertex[vindex].z = -1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(0.0f, 0.0f, -1.0f));
            mTexCoords.push_back( vec2(u,v) );

            vindex++;
        }
    }
    
    face = 3; /* normal is negative x */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = -1.0f;
            mVertex[vindex].y = (float) j * (1.0f / (float) subdivisions) * 2.0f - 1.0f;
            mVertex[vindex].z = (float) i * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(-1.0f, 0.0f, 0.0f));
            mTexCoords.push_back( vec2(u,v) );

            vindex++;
        }
    }
    
    face = 4; /* normal is positive y */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = (float) i * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mVertex[vindex].y = 1.0f;
            mVertex[vindex].z = (float) j * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(0.0f, 1.0f, 0.0f));
            mTexCoords.push_back( vec2(u,v) );

            vindex++;
        }
    }
    
    face = 5; /* normal is negative y */
    vindex = face * (subdivisions + 1) * (subdivisions + 1);
    for (i = 0; i < subdivisions + 1; i++) {
        for (j = 0; j < subdivisions + 1; j++) {
            float v = i / float(subdivisions);
            float u = j / float(subdivisions);
            mVertex.push_back( vec3(0) );
            mVertex[vindex].x = (float) i * (1.0f / (float) subdivisions) * -2.0f + 1.0f;
            mVertex[vindex].y = -1.0f;
            mVertex[vindex].z = (float) j * (1.0f / (float) subdivisions) * 2.0f - 1.0f;
            mNormals.push_back( vec3(0) );
            mNormals[vindex] = normalize(vec3(0.0f, -1.0f, 0.0f));
            mTexCoords.push_back( vec2(u,v) );

            vindex++;
        }
    }
    
    for (i = 0; i < 6; i++){
        make_unit_cube_triangles(i, subdivisions);
    }
    
    //set_flat_shading_vertex_normals();
    
    //m->radius = mesh_compute_radius(m);
    //mesh_graph_dev_init(m);
    
}


/* An attempt at an empirical solution, sampling rather than computing analytically.
 * This only works with a spherified cube.
 */
void CubeSphere::sample_spherical_cubemap_tangent_and_bitangent()
{
    
    vec3 tsample0, tsample1, tsample2, tangent;
    float epsilon = 0.001;
    int face;
    
    const float epsilon_factor[6][6] = {
        {  1.0,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f, }, /* face 0 */
        {  0.0f, 0.0f, -1.0f, 0.0f, -1.0f,  0.0f, }, /* face 1 */
        { -1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, }, /* face 2 */
        {  0.0f, 0.0f,  1.0f, 0.0f, -1.0f,  0.0f, }, /* face 3 */
        {  1.0f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f, }, /* face 4 */
        {  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f, }, /* face 5 */
    };
    
    /* This algorithm will have problems for triangles which have vertices which
     * are on different faces of the cubemap.  Luckily there are no such vertices
     * in the case of a spherified cube with duplicated edge vertices.
     */
    
    int triangles_per_face = numTriangles / 6;
    
    for (uint i = 0; i < numTriangles; i++) {
            face = i / triangles_per_face;
        
            vec3 normal0 = normalize( mTriangles[i].v0);
            vec3 normal1 = normalize( mTriangles[i].v1);
            vec3 normal2 = normalize( mTriangles[i].v2);

        //v2 = (-y1, x1, z1)

            /* Based on which face of cubemap we're on, figure which coords
             * play roles of x and y in calculation of tangent and bitangent
             */
            tsample0.x = normal0.x + epsilon_factor[face][0] * epsilon;
            tsample0.y = normal0.y + epsilon_factor[face][1] * epsilon;
            tsample0.z = normal0.z + epsilon_factor[face][2] * epsilon;

            tsample1.x = normal1.x + epsilon_factor[face][0] * epsilon;
            tsample1.y = normal1.y + epsilon_factor[face][1] * epsilon;
            tsample1.z = normal1.z + epsilon_factor[face][2] * epsilon;

            tsample2.x = normal2.x + epsilon_factor[face][0] * epsilon;
            tsample2.y = normal2.y + epsilon_factor[face][1] * epsilon;
            tsample2.z = normal2.z + epsilon_factor[face][2] * epsilon;

            
            tsample0 = normalize(tsample0);
            tsample1 = normalize(tsample1);
            tsample2 = normalize(tsample2);
        
        
        
            
            /*vec3_sub(&tangent, &tsample, &normal);
            vec3_normalize_self(&tangent);
            
            m->t[i].vtangent[j].x = tangent.v.x;
            m->t[i].vtangent[j].y = tangent.v.y;
            m->t[i].vtangent[j].z = tangent.v.z;*/
        
    }
}


void CubeSphere::unit_spherified_cube( bool change )
{
    vec3 v, s, n;
    
    /* Normalize all the vertices to turn the cube into a sphere */
    for (uint i = 0; i < numVertices; i++) {
        
        v = mVertex[i];
        
        float x2 = v.x * v.x;
        float y2 = v.y * v.y;
        float z2 = v.z * v.z;

        s.x = v.x * sqrt(1.0f - y2 / 2.0f - z2 / 2.0f + y2 * z2 / 3.0f);
        s.y = v.y * sqrt(1.0f - x2 / 2.0f - z2 / 2.0f + x2 * z2 / 3.0f);
        s.z = v.z * sqrt(1.0f - x2 / 2.0f - y2 / 2.0f + x2 * y2 / 3.0f);
        
        if(change)
        {
            v = mVertex[i];
            n = mNormals[i];
            
            mNormals[i] = mVertex[i];
            mVertex[i] = normalize(v);
            
        }

        else{
            mVertex[i] = s;
            mNormals[i] = mVertex[i];
        }
    
    }
    
    //mesh_sample_spherical_cubemap_tangent_and_bitangent(m);
}


void CubeSphere::calculateTangents( size_t numIndices, const uint16_t *indices, size_t numVertices, const vec3 *positions, const vec3 *normals, const vec2 *texCoords, vector<vec3> *resultTangents )
{
    
    if( resultTangents )
        resultTangents->assign( numVertices, vec3( 0 ) );
    
    for( size_t i = 0; i < mIndices.size(); ++i ) {
        
        uint16_t index0 = indices[i];
        uint16_t index1 = indices[i + 1];
        uint16_t index2 = indices[i + 2];
        
        const vec3 &v0 = positions[index0];
        const vec3 &v1 = positions[index1];
        const vec3 &v2 = positions[index2];
        
        const vec2 &w0 = vec2( texCoords[index0] );
        const vec2 &w1 = vec2( texCoords[index1] );
        const vec2 &w2 = vec2( texCoords[index2] );
        
        float x1 = v1.x - v0.x;
        float x2 = v2.x - v0.x;
        float y1 = v1.y - v0.y;
        float y2 = v2.y - v0.y;
        float z1 = v1.z - v0.z;
        float z2 = v2.z - v0.z;
        
        float s1 = w1.x - w0.x;
        float s2 = w2.x - w0.x;
        float t1 = w1.y - w0.y;
        float t2 = w2.y - w0.y;
        
        float r = (s1 * t2 - s2 * t1);
        if( r != 0.0f ) r = 1.0f / r;
        
        vec3 tangent( (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r );
        
        (*resultTangents)[index0] += tangent;
        (*resultTangents)[index1] += tangent;
        (*resultTangents)[index2] += tangent;
    }
    
    for( size_t i = 0; i < numVertices; ++i ) {
        vec3 normal = normals[i];
        vec3 tangent = (*resultTangents)[i];
        (*resultTangents)[i] = ( tangent - normal * dot( normal, tangent ) );
        
        float len = length2( (*resultTangents)[i] );
        if( len > 0.0f )
            (*resultTangents)[i] /= sqrt( len );
    }
}




