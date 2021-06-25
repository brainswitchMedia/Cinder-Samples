#include "DeformedSphere.h"

using namespace ci;
using namespace std;
using namespace cinder;

DeformedSphere::DeformedSphere(): mRadius( 2.0f ), mCenter(vec3( 0, 0, 0 )), mSubdivisions(84)
{
    mRandCoef = 0.2f;
    mRadiusSizeCoef = 1.0f;
}


gl::VboMeshRef DeformedSphere::create()
{
    float lastPosy = 0;
    bool nextIMGLigne = false;
    

    mNumSegments = mSubdivisions;
    if( mNumSegments < 4 )
        mNumSegments = std::max( 12, (int)math<double>::floor( mRadius * float(M_PI * 2) ) );
    
    mNumRings = ( mNumSegments >> 1 ) + 1;
    mNumSegments += 1;
    
    app::console() <<  "mRadius " << mRadius << " // ";
    app::console() <<  "mCenter " << mCenter << " // ";
    app::console() <<  "mNumRings " << mNumRings << " // ";
    app::console() <<  "mNumSegments " << mNumSegments << " // ";
    
    
    std::vector<vec3> positionsMemory;
    std::vector<vec3> randPositionMemory;
    
    int vectorpIter = 0;
    
    float cyclevalue = M_PI/ 8.0f;
    std::vector<float> cycle;
    for (int i = 0; i < 8; i++)
    {
        cycle.push_back(math<float>::sin(cyclevalue*i));
        std::cout << " -cyclevalue- " << math<float>::sin(cyclevalue*i) << std::endl;
    }
    
    mPosCoords.resize( mNumSegments * mNumRings );
    mNormals.resize( mNumSegments * mNumRings );
    mTexCoords.resize( mNumSegments * mNumRings );
    mIndices.resize( (mNumSegments - 1) * (mNumRings - 1) * 6 );
    mColors.resize( mNumSegments * mNumRings );
    
    float ringIncr = 1.0f / (float)( mNumRings - 1 );
    float segIncr = 1.0f / (float)( mNumSegments - 1 );
    
    
    auto vertIt = mPosCoords.begin();
    auto normIt = mNormals.begin();
    auto texIt = mTexCoords.begin();
    auto colorIt = mColors.begin();
    for( int r = 0; r < mNumRings; r++ ) {
        float v = r * ringIncr;
        
        std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
        for( int s = 0; s < mNumSegments; s++ ) {
            float u = 1.0f - s * segIncr;
            float x = math<float>::sin( float(M_PI * 2) * u ) * math<float>::sin( float(M_PI) * v );
            float y = math<float>::sin( float(M_PI) * (v - 0.5f) );
            float z = math<float>::cos( float(M_PI * 2) * u ) * math<float>::sin( float(M_PI) * v );
            
            vec3 pos = vec3( x * mRadius + mCenter.x, y * mRadius + mCenter.y, z * mRadius + mCenter.z );
            
            if( lastPosy == y ) nextIMGLigne = false;
            else nextIMGLigne = true;
            
            lastPosy = y;
            
            bool contains = false;
            std::cout << "----------------------------" << std::endl;
            
            
            if(pos.y != mRadius && pos.y != -mRadius )
            {
                
                if (positionsMemory.size() == 0)
                {
                    positionsMemory.push_back(pos);
                    // small sphere
                    //float rand = mRandCoef * randFloat( -mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y), mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y) );
                    // big sphere
                    float rand = mRandCoef * randFloat( -mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y), mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y) );
                    randPositionMemory.push_back(normalize(pos)*rand);
                    vectorpIter = randPositionMemory.size() - 1;
                }
                
                else
                {
                    for ( int i = 0; i < positionsMemory.size(); i++)
                    {
                        if (pos.x == 0.0f && positionsMemory[i].y == pos.y && positionsMemory[i].z == pos.z )
                        {
                            contains = true;
                            //std::cout << positionsMemory[i] << " // "<<  pos << std::endl;
                            vectorpIter = i;
                            break;
                        }
                    }
                    if(contains == false)
                    {
                        positionsMemory.push_back(pos);
                        float rand = mRandCoef * randFloat( -mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y), mRadius - math<float>::abs(pos.y)*math<float>::abs(pos.y) );
                        randPositionMemory.push_back(normalize(pos)*rand);
                        vectorpIter = randPositionMemory.size() - 1;
                    }
                }
            }
            else
            {
                std::cout <<  pos << std::endl;
            }
            
            if ( pos.y == mRadius || pos.y == -mRadius)
            {
                *vertIt++ = pos * mRadiusSizeCoef;
            }
            else
            {
                *vertIt++ = pos * mRadiusSizeCoef + (mRadius - math<float>::abs(positionsMemory[vectorpIter].y)) * randPositionMemory[vectorpIter];
            }
            
            //*vertIt++ = pos;
            *normIt++ = normalize(vec3( x, y, z ));
            *texIt++ = vec2( u, v );
            *colorIt++ = vec3( x * 0.5f + 0.5f, y * 0.5f + 0.5f, z * 0.5f + 0.5f );
        }
    }
    
    std::map< int, vec3 > newNormals;
    
    auto indexIt = mIndices.begin();
    for( int r = 0; r < mNumRings - 1; r++ ) {
        for( int s = 0; s < mNumSegments - 1 ; s++ ) {
            std::cout << " ++++++++++++++++++++++++++++++++++++++++++++++++++ "<< std::endl;
            *indexIt++ = (uint32_t)(r * mNumSegments + ( s + 1 ));
            *indexIt++ = (uint32_t)(r * mNumSegments + s);
            *indexIt++ = (uint32_t)(( r + 1 ) * mNumSegments + ( s + 1 ));
            vec3 v0 = mPosCoords[(uint32_t)(r * mNumSegments + ( s + 1 ))];
            vec3 v1 = mPosCoords[(uint32_t)(r * mNumSegments + s)];
            vec3 v2 = mPosCoords[(uint32_t)(( r + 1 ) * mNumSegments + ( s + 1 ))];
            
            if (v0.x == 0.0f ) {std::cout << " v00 "<<  v0 << std::endl;}
            if (v1.x == 0.0f ) {std::cout << " v1 "<<  v1 << std::endl;}
            if (v2.x == 0.0f ) {std::cout << " v2 "<<  v2 << std::endl;}
            
            if( math<float>::abs(v0.y) < 2.0 && math<float>::abs(v1.y) < 2.0 && math<float>::abs(v2.y) < 2.0 )
            {
                vec3 e0 = v2 - v0;
                vec3 e1 = v2 - v1;
                vec3 n = normalize(cross(e0, e1));
                
                int vindice = r * mNumSegments + ( s + 1 );
                auto it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
                
                vindice = r * mNumSegments + s;
                it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
                
                vindice = ( r + 1 ) * mNumSegments + ( s + 1 );
                it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
            }
            
            std::cout << " ----------------------------------- " << std::endl;
            
            *indexIt++ = (uint32_t)(( r + 1 ) * mNumSegments + s);
            *indexIt++ = (uint32_t)(( r + 1 ) * mNumSegments + ( s + 1 ));
            *indexIt++ = (uint32_t)(r * mNumSegments + s);
            v0 = mPosCoords[(uint32_t)(( r + 1 ) * mNumSegments + s)];
            v1 = mPosCoords[(uint32_t)(( r + 1 ) * mNumSegments + ( s + 1 ))];
            v2 = mPosCoords[(uint32_t)(r * mNumSegments + s)];
            
            if (v0.x == 0.0f ) {std::cout << " v01 "<<  v0 << std::endl; }
            if (v1.x == 0.0f ) {std::cout << " v1 "<<  v1 << std::endl; }
            if (v2.x == 0.0f ) {std::cout << " v2 "<<  v2 << std::endl; }
            
            if( math<float>::abs(v0.y) < 2.0 && math<float>::abs(v1.y) < 2.0 && math<float>::abs(v2.y) < 2.0 )
            {
                vec3 e0 = v2 - v0;
                vec3 e1 = v2 - v1;
                vec3 n = normalize(cross(e0, e1));
                
                int vindice = ( r + 1 ) * mNumSegments + s;
                auto it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
                
                vindice = ( r + 1 ) * mNumSegments + ( s + 1 );
                it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
                
                vindice = ( r * mNumSegments + s );
                it = newNormals.find( vindice );
                if (it != newNormals.end())
                {
                    std::cout << "normal: " << it->second << "\n";
                    it->second = normalize(it->second + n);
                    mNormals[ (uint32_t)vindice] = it->second;
                }
                else
                {
                    newNormals.insert ( std::pair<int, vec3> ( vindice , n ));
                    mNormals[ (uint32_t)vindice] = normalize( mNormals[ (uint32_t)vindice] + n );
                }
            }
        }
    }
    
    gl::VboMesh::Layout layout;
    layout.attrib( geom::POSITION, 3 );//.usage( GL_DYNAMIC_DRAW );
    layout.attrib( geom::NORMAL, 3 );
    layout.attrib( geom::TANGENT, 3 );
    layout.attrib( geom::TEX_COORD_0, 2 );//.usage( GL_STATIC_DRAW );
    
    calculateTangents( mIndices.size(), mIndices.data(), mPosCoords.size(), mPosCoords.data(), mNormals.data(), mTexCoords.data(), &mTangents );
    
    mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLES, { layout }, mIndices.size() );
    mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
    mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
    mVboMesh->bufferAttrib( geom::TANGENT, mTangents.size() * sizeof( vec3 ), mTangents.data() );
    mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
    mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );

    
    return mVboMesh;
}


void DeformedSphere::setRadius( float v )
{
    mRadius = v;
}


void DeformedSphere::setCenter( const ci::vec3& v )
{
    mCenter = v;
}


void DeformedSphere::setNumSubdivision( const int v )
{
    mSubdivisions = v;
}


void DeformedSphere::calculateTangents( size_t numIndices, const uint16_t *indices, size_t numVertices, const vec3 *positions, const vec3 *normals, const vec2 *texCoords, vector<vec3> *resultTangents )
{
    if( resultTangents )
        resultTangents->assign( numVertices, vec3( 0 ) );
    
    size_t numTriangles = numIndices / 3;
    for( size_t i = 0; i < numTriangles; ++i ) {
        uint16_t index0 = indices[i * 3];
        uint16_t index1 = indices[i * 3 + 1];
        uint16_t index2 = indices[i * 3 + 2];
        
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


