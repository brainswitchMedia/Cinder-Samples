//
//  SphereParticle.h
//  tetraedre3
//
//  Created by Daniel Schweitzer on 30.03.22.
//
//

#pragma once
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class SphereParticle {
    
public:
    
    SphereParticle();
    SphereParticle( vec3 position, vec3 lightPosition, vec3 faceNormal, float distToFaceGravityCenter );
    
    // 3D center position
    vec3    mPosition;
    
    // 3D light positon
    vec3    mLightPosition;
    
    // 3D tetraedre face normal if the sphere is on the face
    // the average normal of the 2 adjacent faces if the point is on an edge
    // the average normal of the 3 adjacent faces if the point is a corner
    vec3    mFaceNormal;
    
    // Distance to the gravity center of the face
    float       mDistToFaceGravityCenter;
};
