//
//  TF_Particles.h
//  deformedSphereMappingV1
//
//  Created by Daniel Schweitzer on 06.11.20.
//
//

#pragma once
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"

#include "cinder/gl/TransformFeedbackObj.h"

using namespace std;
using std::vector;
using namespace cinder;

const int MAX_PARTICLES = 10000;

struct Particle
{
    vec3 vPosition;
    float fLifeTime;
    vec3 vVelocity;
    float fSize;
    float fAge;
    int iType;
};


class TF_Particles
{
public:
    TF_Particles();
    void setup();
    void update( float counter );
    void draw( const CameraPersp &camera, float luminosity, float bloomLuminosity, vec2 invViewportSize, const gl::TextureRef objetcDepthTexture );
    void loadBuffers();
    void loadShaders();
    void loadTexture();
    float grandf( float fMin, float fAdd );
    void setGeneratorProperties( vec3 a_vGenPosition, vec3 a_vGenVelocityMin, vec3 a_vGenVelocityMax, float a_fGenLifeMin, float a_fGenLifeMax, float a_fGenSize, float fEvery, int a_iNumToGenerate );
    
    // CURL PARAM ////////////////////////////////////////////////////
    float                           mNoiseScale = 0.015f;
    float                           mNoise_strength = 0.058f;
    float                           mLength_scale = 0.025f;
    float                           progression_rate = 0.275f;
    float                           rotational_strength = 0.018f;
    float                           rotational_speed_strength = -0.001f;
    float                           rotational_speed_strength2 = 0.0f;
    float                           centripedAttraction = 0.002f;
    float                           velocityDamping = 0.75f;
    // Rendering vanishing the particle with depth
    float                           adjustedZFar = 0.0f;
    
private:
    // Descriptions of particle data layout.
    gl::VaoRef                      mAttributes[2];
    // Buffers holding raw particle data on GPU.
    gl::VboRef                      mParticleBuffer[2];
    //gl::TransformFeedbackObjRef     mTfo[2];
    
    gl::GlslProgRef					mPUpdateGlsl, mPRenderGlsl;
    
    // Current source and destination buffers for transform feedback.
    // Source and destination are swapped each frame after update.
    std::uint32_t                   iCurReadBuffer		= 0;
    
    // Textures
    gl::TextureRef					mParticleTexture;
    
    
    bool m_isFirst = true;
    
    float fElapsedTime;
    float fLastFrameTime = 0.0f;
    float fNextGenerationTime = 2.0f;
    
    vec3 vGenPosition = vec3(0, 0, 0);
    vec3 vGenVelocityMin, vGenVelocityRange;
    vec3 vGenGravityVector;
    vec3 vGenColor;
    
    float fGenLifeMin, fGenLifeRange;
    float fGenSize;
    
    int iNumToGenerate;
    
    gl::QueryRef mQuery;
    int iNumParticles = 1;
};
