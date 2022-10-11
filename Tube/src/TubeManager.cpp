
#include "TubeManager.h"

TubeManager::TubeManager( )
{
    mNumberOfTubes = 0;
    mTubesParams.mTubeLength = 0;
}


void TubeManager::setup( const std::string &pathVert, const std::string &pathFrag, const std::string &pathTexture1, const std::string &pathTexture2 )
{    
    mTubesParams.mIsTubes               = false;
    mTubesParams.mTubeNumber            = 8;
    mTubesParams.mAutoGrowDiam          = true;
    mTubesParams.mAutoGrowLength        = false;
    mTubesParams.mMaxScale0             = 0.0f;
    mTubesParams.mMaxScale1             = 0.7f;
    mTubesParams.mLengthParticleLoc     = 65;
    mTubesParams.mTubeLength            = 20;
    mTubesParams.mVelCoef               = 0.03f;
    mTubesParams.mVelDamping            = 0.97f;
    mTubesParams.mIsStripesType         = 0;
    mTubesParams.mIsBloom               = false;
    mTubesParams.mIsGodRays             = false;
    mTubesParams.mMagnitude1            = 1.0f;
    isTubeClosed                        = true;
    mCurveNumSegs                       = 65;
    
    // Load the texture
    try {
        // Load the textures.
        gl::Texture::Format fmt;
        fmt.setAutoInternalFormat();
        fmt.setWrap( GL_REPEAT, GL_REPEAT );
        mTexture1 = gl::Texture::create( loadImage( loadAsset( pathTexture1 ) ), fmt );
        mTexture2 = gl::Texture::create( loadImage( loadAsset( pathTexture2 ) ), fmt );

	}
	catch( const std::exception &e ) {
        console() << "Could not load texture:" << e.what() << std::endl;
    }
    
    // color shader
    try {
        mTubesColorsShader = gl::GlslProg::create( loadAsset( pathVert ), loadAsset( pathFrag ) );
	}
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Fxaa shader: " << exc.what() );
    }

    // init the static vbo
    Tube::initVBOMesh( mCurveNumSegs );
}


void TubeManager::addTube( int shapeType )
{
    // Tube ///////////////////////////////////////////////
    // Tube param
    mParallelTransport	= true;
	mDrawCurve			= false;
	mDrawFrames			= true;
	mDrawMesh			= true;
	mDrawSlices			= false;
	mShape				= shapeType;
	mWireframe			= true;
    
    // Profile
	switch( mShape ) {
		case 0:
			makeCircleProfile( prof, 0.10f, 20 );
            break;
		case 1:
			makeStarProfile( prof, 0.25f );
            break;
		case 2:
			makeHypotrochoid( prof, 0.25f );
            break;
		case 3:
			makeEpicycloid( prof, 0.25f );
            break;
	}
    
    TubeParticles* objTubeParticle = new TubeParticles();
    mTubeParticles.push_back( objTubeParticle );

    int size = mTubes.size();
    Tube* objTube = new Tube( prof, 0.0f, 0.01f, size );
    mTubes.push_back( objTube );
    
    mNumberOfTubes = mTubes.size();
    //mTubes[size]->mRandom = Rand::randFloat( 0.5f, 1.0f );
    mTubes[size]->mRandom = 1.0f;
    //mTubeParticles[size]->mRandom = Rand::randFloat( 0.8f, 1.0f );
    mTubeParticles[size]->mRandom = 1.0f;

}


void TubeManager::removeTube()
{
    delete mTubes[ mTubes.size() - 1 ];
    mTubes.erase( mTubes.end() - 1 );
    mNumberOfTubes = mTubes.size();
        
    delete mTubeParticles[ mTubeParticles.size() - 1 ];
    mTubeParticles.erase( mTubeParticles.end() - 1 );
}


TubeParticles* TubeManager::getTupeParticle( int i )
{
    return mTubeParticles[i];
}


void TubeManager::update( vector <ci::vec3> &alocParticleEmitters, vec3 rotationVector )
{
    // Number of tubes
    mNumberOfTubes = mTubes.size();
    
	// update tubes if at least 1 tube
    if ( mNumberOfTubes != 0 )
    {
        int numberOfEmitter = alocParticleEmitters.size();
        int tubeToUpdate = 0;
        
        // Update alreday existing tubes ( and add new tubes later )
        if ( mNumberOfTubes < numberOfEmitter ) tubeToUpdate = mNumberOfTubes;
        else tubeToUpdate = numberOfEmitter;
        
        std::vector<ci::vec3> particlesLoc;
        
        for( int i = 0; i < tubeToUpdate; i++ )
        {
            int numberOfParticles = mTubeParticles[i]->GetParticlesNumber();
            mTubeParticles[i]->moveParticles( mTubesParams.mVelDamping, mTubesParams.mVelCoef );
            mTubeParticles[i]->EmitParticles( alocParticleEmitters[i] );

            // At least 3 particles ( points ) to build the PTF
            if ( numberOfParticles > 3 )
            {
                // It's possible to choose how many particle we want to build the Tube ( short or longer tube )
                float tempLengthParticlesLoc = mTubesParams.mLengthParticleLoc;
                if ( tempLengthParticlesLoc > numberOfParticles ) tempLengthParticlesLoc = numberOfParticles;
                
                for( int j = 0; j < tempLengthParticlesLoc; ++j )
                {
                    particlesLoc.push_back( mTubeParticles[i]->GetParticlePosition(j) );
                }
            
                BSpline3f mBSpline = BSpline3f( particlesLoc, 3, false, true );
                mTubes[i]->setBSpline( mBSpline );
                mTubes[i]->sampleCurve();
                mTubes[i]->buildPTF();
            }
            
            // For the first tube
            if ( i == 0 )
            {
                // we construct one time the VBO mesh for all tubes
                mTubes[i]->buildVBOMesh( mTubesParams.mTubeLength, isTubeClosed );
            }
            
            mTubes[i]->buildTube( isTubeClosed );
            mTubes[i]->updateVBOMesh( isTubeClosed, mTubeParticles[i]->mOrigin );
            
            if ( mTubes[i]->mScale1 < mTubesParams.mMaxScale1 * mTubes[i]->mRandom && mTubesParams.mAutoGrowDiam )
            {
                mTubes[i]->mScale1 += 1.0f;
            }
            else
            {
                mTubes[i]->mScale1 = mTubesParams.mMaxScale1 * mTubes[i]->mRandom;
            }
            mTubes[i]->mScale0 = mTubesParams.mMaxScale0;
            
            // mTubeLength can not be > mCurveNumSegs
            if ( mTubesParams.mAutoGrowLength && mTubesParams.mTubeLength < mCurveNumSegs )
            {
                mLengthGrowtimer += 0.5f;
                
                if ( mLengthGrowtimer > 1.0f )
                {
                    mTubesParams.mTubeLength += 1;
                    mLengthGrowtimer = 0.0f;
                }
            }
            
            particlesLoc.clear();
        }
    }
    
    // Add or removes tubes
    if ( mTubesParams.mTubeNumber > mNumberOfTubes )
    {

        while ( mNumberOfTubes < mTubesParams.mTubeNumber )
        {
            addTube( 0 );
        }
    }
    else if ( mTubesParams.mTubeNumber < mNumberOfTubes )
    {
        while ( mNumberOfTubes > mTubesParams.mTubeNumber)
        {
            removeTube();
        }
    }
    
    if ( mTubesParams.mMagnitude1 > 1.0f )
    {
        mTubesParams.mMagnitude1 -= 0.1f;
        mRotationValue += 0.0005f;
    }
    
    //if ( mRotationValue > 1.0f ) mRotationValue -= 0.00001f;
}


void TubeManager::drawTubes( float counter )
{
    const gl::ScopedTextureBind scopedTextureBind0( mTexture1, 0 );
    const gl::ScopedTextureBind scopedTextureBind1( mTexture2, 1 );
    gl::ScopedGlslProg glslScope( mTubesColorsShader );
    mTubesColorsShader->uniform( "uTexGradiant", 0 );
    mTubesColorsShader->uniform( "uTexStripes", 1 );
    mTubesColorsShader->uniform( "uTime", 0.2f * counter );

    int unsigned size =  mTubes.size();
    
    for (int unsigned i=0; i < size; i++ )
    {
        mTubes[i]->drawVBOMesh();
    }
    
    // to test normals
    /*for (int unsigned i=0; i < size; i++ )
    {
        mTubes[i]->drawTangents( mTubesParams.mLen );
    }*/

}


mat3 TubeManager::rotationAlign( vec3 d, vec3 z )
{
    vec3  v = cross( z, d );
    float c = dot( z, d );
    float k = 1.0 / ( 1.0 + c );
    
    mat3 aMat3 = mat3( v.x*v.x*k + c,   v.x*v.y*k + v.z,    v.x*v.z*k - v.y,
                      v.y*v.x*k - v.z,  v.y*v.y*k + c,      v.y*v.z*k + v.x,
                      v.z*v.x*k + v.y,  v.z*v.y*k - v.x,    v.z*v.z*k + c );
    return aMat3;
}
