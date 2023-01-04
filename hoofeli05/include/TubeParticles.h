#pragma once
#include "cinder/Vector.h"
#include "cinder/app/App.h"
#include <vector>


using namespace ci;
using namespace ci::app;
using std::vector;
using std::list;


class TubeParticles {
public:

    TubeParticles();
    ~TubeParticles() {}
    int     GetParticlesNumber();
    vec3    GetParticlePosition( int pos );
    void    moveParticles( float damping, float VelCoef );
    void    EmitParticles( ci::vec3 aOrigin );
    void    AddParticle();
    
    float                       mRandom;
    vec3                        mOrigin;

private:
    std::vector<ci::vec3>       mLoc;
    std::vector<ci::vec3>       mVel;
    int                         mParticleNumber = 65;
    int                         maxParticles = 100;
};
