//
//  SphereParticle.cpp
//  tetraedre3
//
//  Created by Daniel Schweitzer on 30.03.22.
//
//

#include "SphereParticle.h"


SphereParticle::SphereParticle()
{
}


SphereParticle::SphereParticle( vec3 position, vec3 lightPosition, vec3 faceNormal, float distToFaceGravityCenter, int type )
{
    mPosition = position;
    mLightPosition = lightPosition;
    mFaceNormal = faceNormal;
    mDistToFaceGravityCenter = distToFaceGravityCenter;
    mParticleType = type;
}
