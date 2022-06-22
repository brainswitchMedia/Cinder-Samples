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
        p.Size = 0.0f;
        p.Normal = particles[i].mNormal;
        p.SubNormal1 = particles[i].mSubNormal1;
        p.DistToFaceGravityCenter = particles[i].mDistToFaceGravityCenter;
        p.Dist_To_Cormer = particles[i].mDist_To_Cormer;
        p.Cameradistance = -1.0f;
        p.Color = particles[i].mColor;
        mParticles.push_back( p );
    }
    
    // Create instance data
    std::vector<InstanceData> positions( particlesSize );
    mInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof( InstanceData ), positions.data(), GL_DYNAMIC_DRAW );
    
    geom::BufferLayout instanceDataLayout;
    //instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, sizeof( InstanceData ), offsetof( InstanceData, position ), 1 /* per instance*/ );
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, sizeof( InstanceData ), offsetof( InstanceData, particlesData ), 1 /* per instance*/ );
    instanceDataLayout.append( geom::Attrib::CUSTOM_1, 16, sizeof( InstanceData ), offsetof( InstanceData, instanceMatrix ), 1 /* per instance*/ );
    

    // Add our instance data buffer to the vbo
    if ( meshType == 0 ) mesh = gl::VboMesh::create( geom::Sphere().subdivisions( 0 ) );
    else if ( meshType == 1 ) mesh = gl::VboMesh::create( geom::Cone() );
    else if ( meshType == 2 ) mesh = gl::VboMesh::create( geom::Capsule().subdivisionsAxis( 10 ).subdivisionsHeight( 12 ) );

    mesh->appendVbo( instanceDataLayout, mInstanceDataVbo );
    
    gl::Batch::AttributeMapping mapping;
    //mapping[geom::Attrib::CUSTOM_0] = "aInstancePosition";
    mapping[geom::Attrib::CUSTOM_0] = "aInstanceParticlesData";
    mapping[geom::Attrib::CUSTOM_1] = "aInstanceMatrix";
    
    mBatchParticles = gl::Batch::create( mesh, mShaderInstancedParticles, mapping );
    mBatchParticles->getGlslProg()->uniform( "uTexSpectrum_Mode1", 0 );
    mBatchParticles->getGlslProg()->uniform( "uTexSpectrum_Mode2", 1 );
    mBatchParticles->getGlslProg()->uniform( "uTexGradiant_00", 2 );
    mBatchParticles->getGlslProg()->uniform( "uTexGradiant_01", 3 );
    mBatchParticles->getGlslProg()->uniform( "uTexGradiant_02", 4 );
    mBatchParticles->getGlslProg()->uniform( "uTexGradiant_03", 5 );
}


void InstanciedParticles::update( float freq1, float freq2, float waveCoef, float sizeCoef, int drawingMode, mat4 &sceneRotation, vec2 fboSize, float time )
{
    // Update InstancedData
    InstanceData* data = (InstanceData*) mInstanceDataVbo->map( GL_WRITE_ONLY );
    
    for( vector<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
        
        float size = 2.0f * pow( 1.0f + 0.5f* ( 1.0f + sin( 3.1415f * p->Dist_To_Cormer * freq2 + freq1 * 3.1415f * time )), 1.5f );
        
        float widthCoef = p->DistToFaceGravityCenter;
        float heightCoef = 0.0f;
        
        // Compute particle size
        if ( drawingMode == 1 ) heightCoef =  pow(sin( HALFPHI * widthCoef ) , 15.0) * waveCoef;
        else heightCoef =  pow(sin( HALFPHI * widthCoef ) , 5.0) * waveCoef;
        
        float colorIndex = 1.0f;
        
        // particle we don't want to draw
        if( p->Color == 5 )
        {
            size = 0.0f;
            widthCoef = 0.0f;
            heightCoef = 0.0f;
            colorIndex = 0.0f;
            p->InitialPosition = vec3( 0.0, 0.0, 0.0 );
        }
        
        mat4 particleRotation = rotationAlign( p->Normal, glm::vec3(0.0, 1.0f, 0.0f));
        
        glm::mat4 modelMatrix = glm::mat4( 1.0f );
        modelMatrix = modelMatrix * sceneRotation;
        modelMatrix = glm::translate( modelMatrix, p->InitialPosition );
        
        
        // Translate
        if ( drawingMode == 0 ) modelMatrix = glm::translate( modelMatrix, heightCoef * size * p->Normal );
        else if ( drawingMode == 1 ) modelMatrix = glm::translate( modelMatrix, ( heightCoef + sizeCoef * size ) * p->Normal );
        else if ( drawingMode == 2 ) modelMatrix = glm::translate( modelMatrix, heightCoef * p->Normal );
        else if ( drawingMode == 3 ) modelMatrix = glm::translate( modelMatrix, 0.5f * heightCoef * p->Normal );
        
        // Rotate
        // If Normal parallel to vec3( 0.0, -1.0f, 0.0f )
        if ( p->Normal == vec3( 0.0, -1.0f, 0.0f ) )
        {
            // subdivide in 2 rotations
            mat4 particleRotation0 = rotationAlign( p->SubNormal1, glm::vec3(0.0, 1.0f, 0.0f));
            mat4 particleRotation1 = rotationAlign( p->Normal, p->SubNormal1 );
            modelMatrix = modelMatrix * particleRotation0;
            modelMatrix = modelMatrix * particleRotation1;
        }
        else{
            modelMatrix = modelMatrix * particleRotation;
        }
        
        // Scale
        if ( drawingMode == 0 ) modelMatrix = glm::scale( modelMatrix, vec3( 0.005f + 0.04f * widthCoef,  0.01f+ heightCoef * size, 0.005f + 0.04f * widthCoef ) );
        else if ( drawingMode == 1 ) modelMatrix = glm::scale( modelMatrix, vec3( 0.005f + 0.04f * widthCoef, 0.01f + heightCoef + sizeCoef * size, 0.005f + 0.04f * widthCoef ) );
        else if ( drawingMode == 2 ) modelMatrix = glm::scale( modelMatrix, vec3( 0.005f + 0.04f * widthCoef, 0.01f + heightCoef, 0.005f + 0.04f * widthCoef ) );
        else if ( drawingMode == 3 ) modelMatrix = glm::scale( modelMatrix, vec3( 0.005f + 0.005f*size , 0.01f + heightCoef, 0.005f + 0.005f*size ) );
        
        // Recalculate Position for sorting
        //p->MovedPosition = mat3( modelMatrix ) * p->InitialPosition;
        
        // Update distance to camera for sorting
        //p->Cameradistance = length2( p->MovedPosition - mCamptr->getEyePoint() );
        
        // Update InstancedData
        // Mode 0 and mode 1
        if ( drawingMode == 0 || drawingMode == 1 ) data->particlesData = vec3( 50.0f * heightCoef, 0.0f, 0.0f );
        // Mode 2 and mode 3
        else data->particlesData = vec3( colorIndex * (float)pow( size, 2 ), 0.0f, 0.0f );
        //data->position          = p->InitialPosition;

        data->instanceMatrix = modelMatrix;
        data++;
        ++p;
    }

    mInstanceDataVbo->unmap();

    // Sort the particles for next update if transparency
    //SortParticles();
}



void InstanciedParticles::drawInstanced( float param_DistanceConverstion, float param_Metallic, float param_Roughness, float param_Mode, float param_RedCoef, float param_ColorIndex )
{
    // Bind Textures
    gl::ScopedTextureBind scopedTextureBind0( mTextureSpectrumMode1, 0 );
    gl::ScopedTextureBind scopedTextureBind1( mTextureSpectrumMode2, 1 );
    gl::ScopedTextureBind scopedTextureBind2( mTexture_Gradiant_00, 2 );
    gl::ScopedTextureBind scopedTextureBind3( mTexture_Gradiant_01, 3 );
    gl::ScopedTextureBind scopedTextureBind4( mTexture_Gradiant_02, 4 );
    gl::ScopedTextureBind scopedTextureBind5( mTexture_Gradiant_03, 5 );
    
    // Update uniforms
    mBatchParticles->getGlslProg()->uniform( "uWorldtoLightMatrix", mLightCamptr->getViewMatrix() );
    mBatchParticles->getGlslProg()->uniform( "uDistanceConverstion", param_DistanceConverstion );
    mBatchParticles->getGlslProg()->uniform( "uViewPos", mCamptr->getEyePoint() );
    mBatchParticles->getGlslProg()->uniform( "uMetallic", param_Metallic );
    mBatchParticles->getGlslProg()->uniform( "uRoughness", param_Roughness );
    mBatchParticles->getGlslProg()->uniform( "uLightPosition", mCamptr->getEyePoint() );
    mBatchParticles->getGlslProg()->uniform( "uMode", (int)param_Mode );
    mBatchParticles->getGlslProg()->uniform( "uRedCoef", param_RedCoef );
    mBatchParticles->getGlslProg()->uniform( "uColorIndex", param_ColorIndex );

    
    mBatchParticles->drawInstanced( mParticles.size() );
    
    //console() << "mParticles.size(): " << mParticles.size() << std::endl;

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
    //fmt.setWrap( GL_REPEAT, GL_REPEAT );
    mTextureSpectrumMode1 = gl::Texture::create( loadImage( loadAsset( "textures/spectrumM1.png" ) ), fmt );
    mTextureSpectrumMode2 = gl::Texture::create( loadImage( loadAsset( "textures/spectrumM2.png" ) ), fmt );
    mTexture_Gradiant_00 = gl::Texture::create( loadImage( loadAsset( "textures/gradiant0.jpg" ) ), fmt );
    mTexture_Gradiant_01 = gl::Texture::create( loadImage( loadAsset( "textures/gradiant1.jpg" ) ), fmt );
    mTexture_Gradiant_02 = gl::Texture::create( loadImage( loadAsset( "textures/gradiant2.jpg" ) ), fmt );
    mTexture_Gradiant_03 = gl::Texture::create( loadImage( loadAsset( "textures/gradiant3.jpg" ) ), fmt );
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

