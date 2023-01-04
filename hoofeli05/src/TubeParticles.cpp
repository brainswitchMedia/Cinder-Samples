#include "TubeParticles.h"


TubeParticles::TubeParticles()
{
    mOrigin = vec3( 0.0f, 0.0f, 0.0f );
}


void TubeParticles::moveParticles( float damping, float VelCoef  )
{
    int particleNB = mLoc.size();
    
    for( int i = 0; i<particleNB; i++ )
    {
        mLoc[i] = ( mLoc[i] + mVel[i] * VelCoef * mRandom );
        mVel[i] = mVel[i] * damping;
    }
}


void TubeParticles::EmitParticles( ci::vec3 aOrigin )
{
    if( mLoc.size() < mParticleNumber )
    {
        mLoc.insert ( mLoc.begin(), aOrigin );
        mVel.insert ( mVel.begin(), normalize( aOrigin ) );
        
        mOrigin = aOrigin;
    }
    else if ( mLoc.size() > mParticleNumber )
    {
        mLoc.pop_back();
        mVel.pop_back();
    }
    else if( mLoc.size() == mParticleNumber )
    {
        for( int i = mParticleNumber - 1; i>0; i-- )
        {
            mLoc[i] = mLoc[i-1];
            mVel[i] = mVel[i-1];
        }
        mLoc[0] = mOrigin = aOrigin;
        mVel[0] = normalize( aOrigin );
    }
}


int TubeParticles::GetParticlesNumber()
{
    return mLoc.size();
}


vec3 TubeParticles::GetParticlePosition( int pos )
{
    return mLoc[pos];
}


void TubeParticles::AddParticle()
{
    mParticleNumber += 1;
}
