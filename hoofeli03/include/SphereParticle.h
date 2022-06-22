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
    SphereParticle( vec3 position );
    
    // 3D center position
    vec3        mPosition;
    
    // 3D Cube corner position outside the tetraetdre triangle face
    vec3        mCubeOppositeCornerPosition;
    
    // 0 on a tetraedre face, 1 on a tetraedre edge, 2 on a tetraedre corner
    int         mSphereType;
    
    // 3D tetraedre face normal if the sphere is on the face
    // the average normal of the 2 adjacent faces if the point is on an edge
    // the average normal of the 3 adjacent faces if the point is a corner
    vec3        mNormal;
    
    // if mNormal = vec3( 0.0, -1.0, 0.0 ) the particle can not be rotated with rotationAlign (rotation axe and destination are parallel )
    // -> cut the rotation in 2 sub rotations with an intermediate rotations
    vec3        mSubNormal1;
    
    // vec( -1, -1 ) if the sphere is not on a edge
    vec2        mTetraedreEdgeAdjacentFaces;
    
    // vec( -1, -1 ) if the sphere is not on a corner
    vec3        mTetraedreCornerAdjacentFaces;
    
    // 3D tetraedre trianglar face
    int         mTetraedreFace;
    
    // Distance to the gravity center of the face
    float       mDistToFaceGravityCenter;
    
    // Distance to the closest centerd unit cube corner
    float       mDist_To_Cormer;
    
    // Shortest distance to the Tetraedre Heigth
    float       mDist_To_Tetraedre_Heigth;
    
    // Distance from the projected Position on the Heigth to the Corner
    float       mDist_Projected_Position_To_Corner;
    
    // Sphere color
    int         mColor;
    
    // Sphere particles can be organised in lines from one tetraedre corner to the base tetraedre triangle
    /*      . line1
     ... line2
     ..... line3
     */
    uint16_t    mLineNumber;
};
