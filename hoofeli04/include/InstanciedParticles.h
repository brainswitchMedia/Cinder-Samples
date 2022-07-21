//
//  Header.h
//  TransformFeedbackInstanciedParticles
//
//  Created by Daniel Schweitzer on 02.05.19.
//
//
#pragma once
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "SphereParticle.h"

#define HALFPHI 3.14159265359f / 2.0f

class InstanciedParticles {
public:
    
    // CPU representation of a particle
    struct Particle{
        vec3        InitialPosition;
        vec3        MovedPosition;
        vec3        Normal;
        vec3        LightPosition;
        float       Cameradistance;
        float       DistToFaceGravityCenter;
        
        // operator  to sort semi-transparent objects from back to front
        bool operator < (const Particle& particle) const
        {
            return ( Cameradistance > particle.Cameradistance );
        }
    };
    
    // Instance Data for Insctanced renderering
    typedef struct InstanceData {
        vec3 position;
        vec3 lightPosition;
        mat4 instanceMatrix;
    } InstanceData;
    
    InstanciedParticles();
    void setup( CameraPersp *cameraPersp, CameraPersp *lightCameraPersp, const vector<SphereParticle> &particles, uint8_t meshType );
    void update( float sizeCoef, mat4 &sceneRotation, vec2 fboSize, vec3 cameraPosition, bool translate, bool adjustSize  );
    void drawInstanced(float param_DistanceConverstion, float param_Brightness, float lightPower, float translulencyPower, int param_Mode, float time );

    private:
    void SortParticles();
    void loadShaders();
    void loadTextures();
    mat4 rotationAlign( vec3 d, vec3 z );
    vec3 slerp( vec3 &start, vec3 &end, float percent );
    
    // Particles
    vector<Particle>            mParticles;
    
    // Camera
    CameraPersp                 *mCamptr = nullptr;
    CameraPersp                 *mLightCamptr = nullptr;
    
    // Instanced Part
    gl::GlslProgRef             mShaderInstancedParticles;
    gl::BatchRef                mBatchParticles;
    gl::VboRef                  mInstanceDataVbo;
    
    // Textures
    gl::TextureRef              mTextureStripes;


};
