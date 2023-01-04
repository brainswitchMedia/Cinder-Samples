
#include "Tube.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;

ci::gl::VboMesh::Layout     Tube::mLayout;
std::vector<uint16_t>       Tube::mIndices;
std::vector<ci::vec2>       Tube::mTexCoords;
int                         Tube::mNumSegs = 20;
int                         Tube::mLength = 0;
bool                        Tube::mVBOIsUpdated = false;
bool                        Tube::mLastIsTubeClosed = true;
std::vector<float>          Tube::mProfileEaseOutExpoCorrectionVal;


void addQuadToMesh( TriMesh& mesh, const vec3& P0, const vec3& P1, const vec3& P2, const vec3& P3 )
{
    mesh.appendPosition( P0 );
    mesh.appendPosition( P1 );
    mesh.appendPosition( P2 );
    mesh.appendPosition( P3 );
    int vert0 = mesh.getNumVertices() - 4;
    int vert1 = mesh.getNumVertices() - 1;
    int vert2 = mesh.getNumVertices() - 2;
    int vert3 = mesh.getNumVertices() - 3;
    mesh.appendTriangle( vert0, vert1, vert3 );
    mesh.appendTriangle( vert3, vert1, vert2 );
}


Tube::Tube()
: mScale0( 0.0f ),
mScale1( 0.0f )
{
    makeCircleProfile( mProf );
}


Tube::Tube( const Tube& obj )
: mBSpline( obj.mBSpline ),
mProf( obj.mProf ),
mScale0( obj.mScale0 ),
mScale1( obj.mScale1 ),
mPs( obj.mPs ),
mTs( obj.mTs ),
mFrames( obj.mFrames )
{
}


Tube::Tube( const BSpline3f& bspline, const std::vector<vec3>& prof )
: mBSpline( bspline ),
mProf( prof ),
mScale0( 0.0f ),
mScale1( 0.0f )
{
}


Tube::Tube( const std::vector<vec3>& prof, float aScale0, float aScale1, int tubenum )
: mProf( prof ),
mScale0( aScale0 ),
mScale1( aScale1 ),
mTubeNum( tubenum )
{
    
}

Tube& Tube::operator=( const Tube& rhs )
{
    if( &rhs != this ) {
        mBSpline	= rhs.mBSpline;
        mProf		= rhs.mProf;
        mScale0		= rhs.mScale0;
        mScale1		= rhs.mScale1;
        mPs			= rhs.mPs;
        mTs			= rhs.mTs;
        mFrames		= rhs.mFrames;
    }
    return *this;
}


void Tube::sampleCurve()
{
    mPs.clear();
    mTs.clear();
    
    float dt = 1.0f/(float)mNumSegs;
    for( int i = 0; i < mNumSegs; ++i ) {
        float t = i*dt;
        
        vec3 P = mBSpline.getPosition( t );
        mPs.push_back( P );
        
        vec3 T = mBSpline.getDerivative( t );
        mTs.push_back( normalize( T ) );
    }
}


void Tube::buildPTF()
{
    mFrames.clear();
    int n = mPs.size();
    
    // Make sure we have at least 3 points because the first frame requires it
    if( n >= 3 ) {
        mFrames.resize( n );
        
        // Make the parallel transport frame
        mFrames[0] = firstFrame( mPs[0], mPs[2], vec3(0.0f, 1.0f, 0.0f) );
        
        // Make the remaining frames - saving the last
        for( int i = 1; i < n - 1; ++i ) {
            vec3 prevT = mTs[i - 1];
            vec3 curT  = mTs[i];
            mFrames[i] = nextFrame( mFrames[i - 1], mPs[i - 1], mPs[i], prevT, curT );
        }
        
        // Make the last frame
        mFrames[n - 1] = lastFrame( mFrames[n - 2], mPs[n - 2], mPs[n - 1] );
    }
}


void Tube::buildFrenet()
{
    mFrames.clear();
    
    int n = mPs.size();
    mFrames.resize( n );
    
    for( int i = 0; i < n; ++i ) {
        vec3 p0, p1, p2;
        if( i < (n - 2) ) {
            p0 = mPs[i];
            p1 = mPs[i + 1];
            p2 = mPs[i + 2];
        }
        else if( i == (n - 2) ) {
            p0 = mPs[i - 1];
            p1 = mPs[i];
            p2 = mPs[i + 1];
        }
        else if( i == (n - 1) ) {
            p0 = mPs[i - 3];
            p1 = mPs[i - 2];
            p2 = mPs[i - 1];
        }
        
        
        vec3 t = normalize( p1 - p0 );
        vec3 n = normalize( cross( t, p2 - p0 ) );
        if( length( n ) == 0.0f ) {
            int i = fabs( t[0] ) < fabs( t[1] ) ? 0 : 1;
            if( fabs( t[2] ) < fabs( t[i] ) )
                i = 2;
            
            vec3 v( 0.0f, 0.0f, 0.0f );
            v[i] = 1.0;
            n = normalize( cross( t, v ) );
        }
        vec3 b = cross( t, n );
        
        mat4& m = mFrames[i];
        m[0][0] = b.x;
        m[1][0] = b.y;
        m[2][0] = b.z;
        m[3][0] = 0;
        
        m[0][1] = n.x;
        m[1][1] = n.y;
        m[2][1] = n.z;
        m[3][1] = 0;
        
        m[0][2] = t.x;
        m[1][2] = t.y;
        m[2][2] = t.z;
        m[3][2] = 0;
        
        m[0][3] = mPs[i].x;
        m[1][3] = mPs[i].y;
        m[2][3] = mPs[i].z;
        m[3][3] = 1;
    }
}


void Tube::initVBOMesh( int numSegs )
{
    mNumSegs = numSegs;
    mLayout.attrib( geom::POSITION, 3 ).usage( GL_DYNAMIC_DRAW );
    mLayout.attrib( geom::NORMAL, 3 ).usage( GL_DYNAMIC_DRAW );
    mLayout.attrib( geom::TANGENT, 3 ).usage( GL_DYNAMIC_DRAW );
    mLayout.attrib( geom::TEX_COORD_0, 2 ).usage( GL_STATIC_DRAW );
}


void Tube::buildMesh( ci::TriMesh *tubeMesh, int lenght )
{
    if( ( mPs.size() != mFrames.size() ) || mFrames.size() < 3 || mProf.empty() )
        return;
    
    tubeMesh->clear();
    
    //OnLy for drawNs
    mNs.clear();
    
    int mLenght = 0;
    
    if ( lenght > mPs.size() ) mLenght = mPs.size();
    else mLenght = lenght;
    
    for( int i = 0; i < mLenght - 1; ++i ) {
        mat4 mat0 = mFrames[i];
        mat4 mat1 = mFrames[i + 1];
        
        float r0 = sin( (float)( i + 0 ) / (float)( mLenght - 1 ) * 3.141592f );
        float r1 = sin( (float)( i + 1 ) / (float)( mLenght - 1 ) * 3.141592f );
        
        //float r0 = 1.0 - (float)(i + 0 )/(float)(mLenght - 1);
        //float r1 = 1.0 - (float)(i + 1 )/(float)(mLenght - 1);
        
        float rs0 = ( mScale1 - mScale0 ) * r0 + mScale0;
        float rs1 = ( mScale1 - mScale0 ) * r1 + mScale0;
        
        for( int ci = 0; ci < mProf.size(); ++ci ) {
            int idx0 = ci;
            int idx1 = ( ci == ( mProf.size() - 1 ) ) ? 0 : ci + 1;
            
            vec3 P0 = vec3 ( mat0 * vec4( mProf[idx0] * rs0, 1 ) );
            vec3 P1 = vec3 ( mat0 * vec4( mProf[idx1] * rs0, 1 ) );
            vec3 P2 = vec3 ( mat1 * vec4( mProf[idx1] * rs1, 1 ) );
            vec3 P3 = vec3 ( mat1 * vec4( mProf[idx0] * rs1, 1 ) );
            
            //OnLy for drawNs
            mNs.push_back( P0 ); // => Normal = mPs[i] - P0
            mNs.push_back( P1 ); // => Normal = mPs[i] - P1
            //mNs.push_back( P2 ); // => Normal = mPs[i] - P2
            //mNs.push_back( P3 ); // => Normal = mPs[i] - P3
            
            
            tubeMesh->appendNormal( mPs[i] - P0 );
            tubeMesh->appendNormal( mPs[i+1] - P3 );
            tubeMesh->appendNormal( mPs[i+1] - P2 );
            tubeMesh->appendNormal( mPs[i] - P1 );
            
            tubeMesh->appendTexCoord( vec2( (float)idx0 / (float)( mProf.size() ), 1.0f - (float)i / (float)( mPs.size() - 1 )   ) );
            tubeMesh->appendTexCoord( vec2( (float)idx0 / (float)( mProf.size() ), 1.0f - (float)(i+1) / (float)( mPs.size() - 1 )   ) );
            
            tubeMesh->appendTexCoord( vec2( (float)( idx0 + 1 ) / (float)( mProf.size() ), 1.0f - (float)( i + 1 ) / (float)( mPs.size() - 1 )  ) );
            tubeMesh->appendTexCoord( vec2( (float)( idx0 + 1 ) / (float)( mProf.size() ) , 1.0f - (float)i / (float)( mPs.size() - 1 ) ) );
            
            addQuadToMesh( *tubeMesh, P0, P3, P2, P1 );
        }
    }
}


/*
 // Type: TRUE
 On ferme le tube par un vertex supplementaire pour palier au probleme des coordonnes de texture devant aller de 0 a 1. Pour des textures procedurales avec le shader.
 
 Exemple: mProf.size() = 6 => 7 vertex
 Coordonnes de textures:
 (1) v6v0 (0)
 (5/6) v5       v1 (1/6)
 (4/6) v4       v2 (2/6)
 v3 (3/6)
 
 Dans la longueur les coordonnees de textures vont de 0 a 1
 
 Vertex:
 v6v0    v1   v2   v3   v4   v5   les vertex v6 et v0 ont les meme coordonnes et les meme normales, le tube est "ferme"
 v13v7   v8   v9   v10  v11  v12  les vertex v13 et v7 ont les meme coordonnes et les meme normales, le tube est "ferme"
 v20v14  v15  v16  v17  v18  v19  les vertex v20 et v14 ont les meme coordonnes et les meme normales, le tube est "ferme"
 
 Ordre des indices:
 
 0,7,1,8,2,9,3,10,4,11,5,12,6,13,0,7,7,14,8,15,9,16,10,17,11,18,12,19,13,20,7,14
 0,7,7 et 7,7,14 sont degeneres et pas affiches
 
 // Type: FALSE
 On ne ferme pas le tube par un vertex supplementaire. Pour des textures importees ou pour afficher par un geometry shader une particule par vertex.
 
 Exemple: mProf.size() = 5 => 6 vertex
 Coordonnes de textures:
 v0 (0)
 (1/3) v5      v1 (1/3)     les coordonnees de texture vont de 0 a 1 puis de nouveau a 0
 (2/3) v4      v2 (2/3)
 v3 (3/3=1)
 
 Dans la longueur les coordonnees de texture sont 0, 1, 0, 1...(i%2), la texture doit etre repetitive
 
 Vertex:
 v0    v1   v2   v3   v4   v5
 v6    v7   v8   v9   v10  v11
 v12   v13  v14  v15  v16  v17
 
 Ordre des indices:
 
 0,6,1,7,2,8,3,9,4,10,5,11,0,6,6,12,7,13,8,14,9,15,10,16,11,17
 0,6,6 et 6,6,12 sont degeneres et pas affiches
 */

// Build vbo mesh must be called one time for all tubes to update vbo
void Tube::buildVBOMesh( int length, bool isTubeClosed )
{
    int mPsSize = mPs.size();
    
    if(  mProf.empty() )
    {
        return;
    }
    
    // Update VBO Buffers if tube profile or tube length is modified
    if ( mLength != length || mLastIsTubeClosed != isTubeClosed )
    {
        mIndices.clear();
        mTexCoords.clear();
        
        if ( length > mPsSize ) mLength = mPsSize;
        else mLength = length;
        
        // Compute new correction values because mLength has been modified
        computeProfileCorrectionTab();
        
        int profileSize = mProf.size();
        
        for( int y = 0; y < mLength - 1; ++y )
        {
            if ( isTubeClosed )
            {
                for( int x = 0; x < profileSize + 1; ++x )
                {
                    mIndices.push_back( y * ( profileSize + 1 ) + x );
                    mIndices.push_back( ( y + 1 ) * ( profileSize + 1 ) + x );
                }
                mIndices.push_back( y * ( profileSize + 1 ) );
                mIndices.push_back( ( y + 1 ) * ( profileSize + 1 ) );
            }
            else
            {
                for( int x = 0; x < profileSize; ++x )
                {
                    mIndices.push_back( y * profileSize + x );
                    mIndices.push_back( ( y + 1 ) * profileSize + x );
                }
                mIndices.push_back( y * profileSize );
                mIndices.push_back( ( y + 1 ) * profileSize );
            }
        }
        
        for( int i = 0; i < mLength; ++i )
        {
            int halhProfileSize = mProf.size()/2;
            int profileSize = mProf.size();
            int PsSize = mLength - 1;
            
            int moduloI = i%2;
            
            if ( isTubeClosed )
            {
                for( int ci = 0; ci < profileSize + 1; ++ci )
                {
                    mTexCoords.push_back( vec2( (float)ci / (float)( profileSize ), (float)i / (float)PsSize ));
                }
            }
            else
            {
                for( int ci = 0; ci < profileSize; ++ci )
                {
                    if ( ci < halhProfileSize + 1 )
                    {
                        mTexCoords.push_back( vec2( (float)ci / (float)( halhProfileSize ), (float)moduloI ));
                    }
                    else
                    {
                        mTexCoords.push_back( vec2( (float)(profileSize - ci )/ (float)( halhProfileSize ), (float)moduloI ));
                    }
                }
            }
        }
        
        int PosCoordSize = mLength * ( profileSize );
        if ( isTubeClosed ) PosCoordSize = mLength * ( profileSize + 1 );
        
        mLastIsTubeClosed = isTubeClosed;
        mVBOIsUpdated = true;
        
    }
    else
    {
        mVBOIsUpdated = false;
    }
}


void Tube::buildTube( bool isTubeClosed )
{
    int mPsSize = mPs.size();
    int profileSize = mProf.size();
    
    if( ( mPsSize != mFrames.size() ) || mFrames.size() < 3 || mProf.empty() || mLength > mPs.size() )
    {
        mTubeIsBuilded = false;
        return;
    }
    
    // Update VBO Buffers if tube profile or tube length was modified
    if ( mVBOIsUpdated || mTubeIsBuilded == false )
    {
        int PosCoordSize = mLength * ( profileSize );
        if ( isTubeClosed ) PosCoordSize = mLength * ( profileSize + 1 );
        
        mPosCoords.clear();
        mNormals.clear();
        mTangents.clear();
        
        mPosCoords.resize( PosCoordSize, vec3( 0 ) );
        mNormals.resize( PosCoordSize, vec3( 0 ) );
        mTangents.resize( PosCoordSize, vec3( 0 ) );
        
        mVboMesh = gl::VboMesh::create( mPosCoords.size(), GL_TRIANGLE_STRIP, { mLayout }, mIndices.size() );
        mVboMesh->bufferAttrib( geom::POSITION, mPosCoords.size() * sizeof( vec3 ), mPosCoords.data() );
        mVboMesh->bufferAttrib( geom::NORMAL, mNormals.size() * sizeof( vec3 ), mNormals.data() );
        mVboMesh->bufferAttrib( geom::TANGENT, mTangents.size() * sizeof( vec3 ), mTangents.data() );
        mVboMesh->bufferAttrib( geom::TEX_COORD_0, mTexCoords.size() * sizeof( vec2 ), mTexCoords.data() );
        mVboMesh->bufferIndices( mIndices.size() * sizeof( uint16_t ), mIndices.data() );
        mTubeIsBuilded = true;
        
        computeProfileCorrectionTab();
    }
}


void Tube::updateVBOMesh( bool isTubeClosed, vec3 origin )
{
    if ( mTubeIsBuilded )
    {
        //mRotateVertex = false;
        mPosCoords.clear();
        mNormals.clear();
        
        for( int i = 0; i < mLength; ++i )
        {
            mat4 mat0 = mFrames[i];
            
            // Shrinked profil at the beginning
            //float r0 = 2.0f * cos( (float)( i ) / (float)( mLength - 1 ) * 1.57079632679f ) * mProfileEaseOutExpoCorrectionVal[i];
            
            // Shrinked profil at the beginning and the end
            //float r0 =  3.0f * sin( (float)( i ) / (float)( mLength - 1 ) * 3.141592f );
            
            // Normal profil
            float r0 = 3.0f * cos( (float)( i ) / (float)( mLength - 1 ) * 1.57079632679f );
            
            // Spike profil
            //float r0 = 1.0 - (float)(i + 0 )/(float)(mLength - 1);
            
            // Used to reduce tube diametre in the middle
            //float rs0 = ( mScale1 - mScale0 ) * r0 + mScale0;
            float rs0 = mScale1 * r0;

            int profileSize = mProf.size();
            if ( isTubeClosed ) profileSize += 1;
            
            int mNormalSizeLastElement = mNormals.size()-1;
            
            for( int ci = 0; ci < profileSize; ++ci )
            {
                int idx0 = ci;
                if ( isTubeClosed ) idx0 = ( ci == ( mProf.size() ) ) ? 0 : ci;
                int idx = idx0;
                
                // Vertices
                mPosCoords.push_back( vec3( mat0 * vec4( mProf[idx] * rs0, 1 ) ) );
                
                // Normals
                if ( i < mLength - 1 ) mNormals.push_back( normalize( vec3( mat0 * vec4( mProf[idx] * rs0 * 2.0f, 1 ) ) - vec3( mat0 * vec4( mProf[idx] * rs0, 1 ) ) ) );
                else mNormals.push_back( mNormals[ mNormalSizeLastElement - profileSize - 1 + ci ]);
            }
        }
        
        // Dynamically update positions, Normals and tangents
        auto mappedAttribPos = mVboMesh->mapAttrib3f( geom::Attrib::POSITION, false );
        auto mappedAttribNor = mVboMesh->mapAttrib3f( geom::Attrib::NORMAL, false );
        auto mappedAttribTan = mVboMesh->mapAttrib3f( geom::Attrib::TANGENT, false );
        
        //console() << "NumVertices : " << mVboMesh->getNumVertices() << std::endl;
        //console() << "mPosCoords : " << mPosCoords.size() << std::endl;
        
        for( int i = 0; i < mVboMesh->getNumVertices(); i++ ) {
            // console() << "mPosCoords[i] : " << mPosCoords[i] << std::endl;
            
            //vec3 &pos = *mappedAttribPos;
            //vec3 &nor = *mappedAttribNor;
            //vec3 &tan = *mappedAttribTan;
            *mappedAttribPos = mPosCoords[i];
            *mappedAttribNor = mNormals[i];
            *mappedAttribTan = mTangents[i];
            ++mappedAttribPos;
            ++mappedAttribNor;
            ++mappedAttribTan;
        }
        mappedAttribPos.unmap();
        mappedAttribNor.unmap();
        mappedAttribTan.unmap();
        
    }
}


void Tube::drawVBOMesh()
{
    if ( mTubeIsBuilded )
    {
        gl::draw( mVboMesh );
    }
}


void Tube::drawTangents( int lenght )
{    
    
}


void Tube::drawNs( float lineWidth, int lenght )
{
    if( mPs.empty() || mNs.empty() )
        return;
    
    int mLenght = 0;
    
    if ( lenght > mPs.size() ) mLenght = mPs.size();
    else mLenght = lenght;
    
    for( int i = 0; i < mLenght - 1; ++i )
    {
        for( int ci = 0; ci < 2*mProf.size(); ++ci )
        {
            gl::lineWidth( 0.5f );
            gl::color( Color( ci/(mProf.size()), 0.0f, 0 ) );
            gl::begin( GL_LINES );
            vec3 point= mNs[ i*2*mProf.size() + ci ];
            gl::vertex( point ); // P0, P1
            vec3 temp= point - mPs[i] ;
            normalize( temp );
            gl::vertex( point+temp );
            gl::end();
        }
    }
}


void Tube::drawPs( float lineWidth )
{
    
    if( mPs.empty() )
        return;
    
    gl::lineWidth( lineWidth );
    gl::begin( GL_LINES );
    for( int i = 0; i < ( mPs.size() - 1 ); ++i ) {
        gl::vertex( mPs[i] );
        gl::vertex( mPs[i + 1] );
    }
    gl::end();
}


void Tube::drawTs( float lineLength , float lineWidth )
{
    if( mPs.empty() || mTs.empty() )
        return;
    
    gl::lineWidth( lineWidth );
    gl::begin( GL_LINES );
    for( int i = 0; i < ( mPs.size() - 1 ); ++i ) {
        gl::vertex( mPs[i] );
        gl::vertex( mPs[i] + mTs[i]*lineLength );
    }
    gl::end();
}


void Tube::drawFrames( float lineLength, float lineWidth )
{
    if( mPs.empty() || mFrames.empty() )
        return;
    
    gl::lineWidth( lineWidth );
    gl::begin( GL_LINES );
    for( int i = 0; i < ( mPs.size() - 1 ); ++i ) {
        
        vec3 xAxis = vec3( mFrames[i] * vec4( 1, 0, 0, 0 ) );
        vec3 yAxis = vec3( mFrames[i] * vec4( 0, 1, 0, 0 ) );
        vec3 zAxis = vec3( mFrames[i] * vec4( 0, 0, 1, 0 ) );
        
        gl::lineWidth( lineWidth );
        gl::color( Color( 1, 0.5f, 0 ) );
        gl::vertex( mPs[i] );
        gl::vertex( mPs[i] + xAxis * lineLength );
        
        gl::color( Color( 1, 0, 1 ) );
        gl::vertex( mPs[i] );
        gl::vertex( mPs[i] + yAxis * lineLength );
        
        gl::lineWidth( 2 * lineWidth );
        gl::color( Color( 0, 1, 1 ) );
        gl::vertex( mPs[i] );
        gl::vertex( mPs[i] + zAxis * lineLength );
    }
    gl::end();
}


mat3 Tube::rotationAlign( vec3 d, vec3 z )
{
    vec3  v = cross( z, d );
    float c = dot( z, d );
    float k = 1.0/(1.0+c);
    
    mat3 aMat3 = mat3( v.x*v.x*k + c,      v.x*v.y*k + v.z,    v.x*v.z*k - v.y,
                      v.y*v.x*k - v.z,    v.y*v.y*k + c,      v.y*v.z*k + v.x,
                      v.z*v.x*k + v.y,    v.z*v.y*k - v.x,    v.z*v.z*k + c );
    return aMat3;
}


void Tube::computeProfileCorrectionTab()
{
    mProfileEaseOutExpoCorrectionVal.resize( mLength, 1.0 );
    
    if ( mLength > 20 )
    {
        float easeValues = floor( mLength / 2.0 );
        
        for( int i = 0; i <= (int)easeValues; ++i )
        {
            float ease = easeOutExpo( (float)i / easeValues );
            // mProfileEaseOutExpoCorrectionVal[i] must be != 0, because of normal calculation
            mProfileEaseOutExpoCorrectionVal[i] = ease;
        }
    }
}


void Tube::drawFrameSlices( float scale )
{
    gl::color( ColorA( 1, 1, 1, 0.15f ) );
    for( int i = 0; i < mFrames.size(); ++i ) {
        gl::pushModelView();
        gl::multModelMatrix( mFrames[i] );
        
        gl::begin( GL_QUADS );
        
        gl::vertex( vec3( -1,  1, 0 ) * scale );
        gl::vertex( vec3(  1,  1, 0 ) * scale );
        gl::vertex( vec3(  1, -1, 0 ) * scale );
        gl::vertex( vec3( -1, -1, 0 ) * scale );
        
        gl::end();;
        gl::popModelView();
    }
    
    gl::color( ColorA( 1, 1, 1, 0.75f ) );
    for( int i = 0; i <  mFrames.size(); ++i ) {
        gl::pushModelView();
        gl::multModelMatrix( mFrames[i] );
        
        gl::begin( GL_LINES );
        
        gl::vertex( vec3( -1,  1, 0 ) * scale );
        gl::vertex( vec3(  1,  1, 0 ) * scale );
        
        gl::vertex( vec3(  1,  1, 0 ) * scale );
        gl::vertex( vec3(  1, -1, 0 ) * scale );
        
        gl::vertex( vec3(  1, -1, 0 ) * scale );
        gl::vertex( vec3( -1, -1, 0 ) * scale );
        
        gl::vertex( vec3( -1, -1, 0 ) * scale );
        gl::vertex( vec3( -1,  1, 0 ) * scale );
        
        gl::end();;
        gl::popModelView();
    }
}


void makeCircleProfile( std::vector<vec3>& prof, float rad, int segments )
{
    prof.clear();
    float dt = 6.28318531f/(float)segments;
    for( int i = 0; i < segments; ++i )
    {
        float t = i*dt;
        prof.push_back( vec3( cos( t ) * rad, sin( t ) * rad, 0 ) );
    }
}


void makeStarProfile( std::vector<vec3>& prof, float rad )
{
    vec3 A(  0.0f,  1.0f, 0.0f );
    vec3 B(  0.5f, -1.0f, 0.0f );
    vec3 C(  0.0f, -0.5f, 0.0f );
    vec3 D( -0.5f, -1.0f, 0.0f );
    
    prof.clear();
    prof.push_back( A );
    prof.push_back( A + (B-A) * 0.3f );
    prof.push_back( A + (B-A) * 0.3f + vec3( 0.75f, 0, 0 ) );
    prof.push_back( A + (B-A) * 0.6f );
    prof.push_back( B );
    prof.push_back( C );
    prof.push_back( D );
    prof.push_back( A + (D-A) * 0.6f );
    prof.push_back( A + (D-A) * 0.3f - vec3( 0.75f, 0, 0 ) );
    prof.push_back( A + (D-A) * 0.3f );
    
    for( int i = 0; i < prof.size(); ++i ) {
        prof[i] *= rad;
    }
}


void makeHypotrochoid( std::vector<vec3>& prof, float rad )
{
    float a = 1;
    float b = 0.142857f;
    float h = b;
    int n = 32;
    prof.clear();
    float dt = 6.28318531f / (float)n;
    for( int i = 0; i < n; ++i ) {
        float t = i * dt;
        float x = ( a - b ) * cos( t ) + h * cos( t * ( a - b ) / b );
        float y = ( a - b ) * sin( t ) - h * sin( t * ( a - b ) / b );
        prof.push_back( vec3( x * rad, y * rad, 0 ) );
    }
}


void makeEpicycloid( std::vector<vec3>& prof, float rad )
{
    float a = 1;
    float b = 0.125f;
    int n = 48;
    prof.clear();
    float dt = 6.28318531f / (float)n;
    for( int i = 0; i < n; ++i ) {
        float t = i * dt;
        float x = ( a + b ) * cos( t ) + b * cos( t * ( a + b ) / b );
        float y = ( a + b ) * sin( t ) + b * sin( t * ( a + b ) / b );
        prof.push_back( vec3( x * rad, y * rad, 0 ) );
    }
}
