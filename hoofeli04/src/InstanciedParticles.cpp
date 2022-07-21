//
//  Nebula.cpp
//  TransformFeedbackInstanciedParticles
//
//  Created by Daniel Schweitzer on 02.05.19.
//
//

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "InstanciedParticles.h"

using namespace ci;
using namespace ci::app;


InstanciedParticles::InstanciedParticles()
{
}


void InstanciedParticles::setup( CameraPersp *cameraPersp, CameraPersp *lightCameraPersp, const vector<SphereParticle> &particles, uint8_t meshType )
{
    // Shader and textures
    loadShaders();
    loadTextures();
    
    mCamptr = cameraPersp;
    mLightCamptr = lightCameraPersp;
    
    gl::VboMeshRef mesh;
    int particlesSize = particles.size();
    
    // Initialise particles
    for( int i = 0; i < particlesSize; i++ ){
        Particle p;
        p.InitialPosition = particles[i].mPosition;
        p.MovedPosition = particles[i].mPosition;
        p.Normal = particles[i].mFaceNormal;
        p.LightPosition = particles[i].mLightPosition;
        p.Cameradistance = -1.0f;
        p.DistToFaceGravityCenter = particles[i].mDistToFaceGravityCenter;

        mParticles.push_back( p );
    }
    
    // Create instance data
    std::vector<InstanceData> positions( particlesSize );
    mInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof( InstanceData ), positions.data(), GL_DYNAMIC_DRAW );
    
    geom::BufferLayout instanceDataLayout;
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, sizeof( InstanceData ), offsetof( InstanceData, position ), 1 /* per instance*/ );
    instanceDataLayout.append( geom::Attrib::CUSTOM_1, 3, sizeof( InstanceData ), offsetof( InstanceData, lightPosition ), 1 /* per instance*/ );
    instanceDataLayout.append( geom::Attrib::CUSTOM_2, 16, sizeof( InstanceData ), offsetof( InstanceData, instanceMatrix ), 1 /* per instance*/ );
    
    // Add our instance data buffer to the vbo
    if ( meshType == 0 ) mesh = gl::VboMesh::create( geom::Sphere().radius( 0.025f ).subdivisions( 30 ) );
    else if ( meshType == 1 ) mesh = gl::VboMesh::create( geom::Cone() );
    else if ( meshType == 2 ) mesh = gl::VboMesh::create( geom::Capsule().subdivisionsAxis( 10 ).subdivisionsHeight( 12 ) );

    mesh->appendVbo( instanceDataLayout, mInstanceDataVbo );
    
    gl::Batch::AttributeMapping mapping;
    mapping[geom::Attrib::CUSTOM_0] = "aInstancePosition";
    mapping[geom::Attrib::CUSTOM_1] = "aInstanceLightPosition";
    mapping[geom::Attrib::CUSTOM_2] = "aInstanceMatrix";

    mBatchParticles = gl::Batch::create( mesh, mShaderInstancedParticles, mapping );
    mBatchParticles->getGlslProg()->uniform( "uTex0", 0 );

}


void InstanciedParticles::update( float sizeCoef, mat4 &sceneRotation, vec2 fboSize, vec3 cameraPosition, bool translate, bool adjustSize )
{
    // Update InstancedData
    InstanceData* data = (InstanceData*) mInstanceDataVbo->map( GL_WRITE_ONLY );
    
    for( vector<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
        
        mat4 particleRotation1 = rotationAlign( p->Normal, glm::vec3( 0.0, 1.0f, 0.0f ) );
        //Only wwork without scene rotation
        //mat4 particleRotation1 = rotationAlign( normalize( cameraPosition ), glm::vec3( 0.0, 1.0f, 0.0f ) );
        // test with rotation does not work
        //mat4 particleRotation2 = rotationAlign( normalize( cameraPosition - p->InitialPosition ), p->Normal );

        glm::mat4 modelMatrix = glm::mat4( 1.0f );
        modelMatrix = modelMatrix * sceneRotation;
        modelMatrix = glm::translate( modelMatrix, p->InitialPosition );
        
        // Translate
        float sizeParticle = sizeCoef;
        if ( adjustSize == true ) sizeParticle = sizeCoef * p->DistToFaceGravityCenter;
        
        if ( translate == true && sizeParticle > 3.16f )
        {
            modelMatrix = glm::translate( modelMatrix, 0.03f * ( sizeCoef - 3.16f ) * p->Normal );
        }

        // Rotate
        //modelMatrix = modelMatrix * particleRotation2;
        modelMatrix = modelMatrix * particleRotation1;

        // Scale
        modelMatrix = glm::scale( modelMatrix, vec3( sizeParticle, sizeParticle, sizeParticle ) );
        
        // Update InstancedData
        data->position          = p->InitialPosition;
        data->lightPosition     = mat3( sceneRotation ) * p->LightPosition;
        data->instanceMatrix    = modelMatrix;
        data++;
        ++p;
    }

    mInstanceDataVbo->unmap();
}


void InstanciedParticles::drawInstanced( float param_DistanceConverstion, float param_Brightness, float lightPower, float translulencyPower, int param_Mode, float time )
{
    // Bind Textures
    gl::ScopedTextureBind scopedTextureBind0( mTextureStripes, 0 );
    
    // Update uniforms
    mBatchParticles->getGlslProg()->uniform( "uWorldtoLightMatrix", mLightCamptr->getViewMatrix() );
    mBatchParticles->getGlslProg()->uniform( "uDistanceConverstion", param_DistanceConverstion );
    mBatchParticles->getGlslProg()->uniform( "uBrightness", param_Brightness );
    mBatchParticles->getGlslProg()->uniform( "uPower", lightPower );
    mBatchParticles->getGlslProg()->uniform( "uMode", param_Mode );
    mBatchParticles->getGlslProg()->uniform( "uTime", 0.2f * time );
    mBatchParticles->getGlslProg()->uniform( "uPowerTranslulencyDist", translulencyPower ); // Without stripes
    mBatchParticles->getGlslProg()->uniform( "uPowerTranslulencyDist", translulencyPower ); // With stripes

    mBatchParticles->drawInstanced( mParticles.size() );
}


// Sort Particles
void InstanciedParticles::SortParticles()
{
    std::sort( mParticles.begin(), mParticles.end());
}


void InstanciedParticles::loadShaders()
{
    try {
        mShaderInstancedParticles = gl::GlslProg::create( loadAsset( "instanced_particles.vert" ), loadAsset( "instanced_particles.frag" ) );
    }
    catch( Exception &exc ) {
        CI_LOG_E( "error loading Instanced Particles shader: " << exc.what() );
    }
}


void InstanciedParticles::loadTextures()
{
    // Load the textures.
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    fmt.setWrap( GL_REPEAT, GL_REPEAT );
    mTextureStripes = gl::Texture::create( loadImage( loadAsset( "textures/stripes.jpg" ) ), fmt );
}


// Rotate an object fron angle d and vector z => Inigo Quilez
mat4 InstanciedParticles::rotationAlign( vec3 d, vec3 z )
{
    vec3  v = cross( z, d );
    float c = dot( z, d );
    float k = 1.0/(1.0+c);
    
    mat4 aMat4 = mat4( v.x*v.x*k + c,       v.x*v.y*k + v.z,    v.x*v.z*k - v.y,    0.0f,
                      v.y*v.x*k - v.z,      v.y*v.y*k + c,      v.y*v.z*k + v.x,    0.0f,
                      v.z*v.x*k + v.y,      v.z*v.y*k - v.x,    v.z*v.z*k + c,      0.0f,
                      0.0f,                 0.0f,               0.0f,               1.0f);
    return aMat4;
}


// Vector slerp
vec3 InstanciedParticles::slerp( vec3 &start, vec3 &end, float percent )
{
    // From cinder 8.3 vector.slerp()
    
    // Dot product - the cosine of the angle between 2 vectors.
    float cosAlpha = dot( start, end );
    
    clamp( cosAlpha, -1.0f, 1.0f );
    
    float alpha = acos( cosAlpha );
    
    float sinAlpha = sin( alpha );
    
    float t1 = sin( ( 1.0 - percent ) * alpha ) / sinAlpha;
    float t2 = sin( percent * alpha ) / sinAlpha;
    
    return start * t1 + end * t2;
    
    // or from internet
    // Special Thanks to Johnathan, Shaun and Geof!
    /*Vector3 Slerp(Vector3 start, Vector3 end, float percent)
     {
     // Dot product - the cosine of the angle between 2 vectors.
     float dot = Vector3.Dot(start, end);
     // Clamp it to be in the range of Acos()
     // This may be unnecessary, but floating point
     // precision can be a fickle mistress.
     Mathf.Clamp(dot, -1.0f, 1.0f);
     // Acos(dot) returns the angle between start and end,
     // And multiplying that by percent returns the angle between
     // start and the final result.
     float theta = Mathf.Acos(dot)*percent;
     Vector3 RelativeVec = end - start*dot;
     RelativeVec.Normalize();     // Orthonormal basis
     // The final result.
     return ((start*Mathf.Cos(theta)) + (RelativeVec*Mathf.Sin(theta)));
     }*/
}

