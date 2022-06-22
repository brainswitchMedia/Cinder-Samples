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


SphereParticle::SphereParticle( vec3 position )
{
    mPosition = position;
    mNormal = vec3( 0, 0, 0 );
    mSphereType = 0;
    mTetraedreEdgeAdjacentFaces = vec2( -1, -1 );
    mTetraedreCornerAdjacentFaces = vec3( -1, -1, -1 );
    mTetraedreFace = -1;
}
