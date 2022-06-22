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
        float       Size;
        vec3        Normal;
        vec3        SubNormal1;
        float       DistToFaceGravityCenter;
        float       Dist_To_Cormer;
        int         Color;
        float       Cameradistance;
        
        // operator  to sort semi-transparent objects from back to front
        bool operator < (const Particle& particle) const
        {
            return (Cameradistance > particle.Cameradistance);
        }
    };
    
    // Instance Data for Insctanced renderering
    typedef struct InstanceData {
        //vec3 position;
        vec3 particlesData;     // x: size, y:not used, z:not used
        mat4 instanceMatrix;
    } InstanceData;
    
    InstanciedParticles();
    void setup( CameraPersp *cameraPersp, CameraPersp *lightCameraPersp, const vector<SphereParticle> &particles, uint8_t meshType );
    void update( float freq1, float freq2, float waveCoef, float sizeCoef, int drawingMode, mat4 &sceneRotation, vec2 fboSize, float time  );
    void drawInstanced(float param_DistanceConverstion, float param_Metallic, float param_Roughness, float param_Mode, float param_RedCoef, float param_ColorIndex );

    private:
    void SortParticles();
    void loadShaders();
    void loadTextures();
    mat4 rotationAlign( vec3 d, vec3 z );
    
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
    gl::TextureRef              mTextureSpectrumMode1;
    gl::TextureRef              mTextureSpectrumMode2;
    gl::TextureRef              mTexture_Gradiant_00;
    gl::TextureRef              mTexture_Gradiant_01;
    gl::TextureRef              mTexture_Gradiant_02;
    gl::TextureRef              mTexture_Gradiant_03;

};
