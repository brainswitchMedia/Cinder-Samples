#include "TF_Particles.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace glm;


TF_Particles::TF_Particles()
{

}



void TF_Particles::setup()
{    
    loadTexture();
    loadShaders();
    loadBuffers();
    
    mQuery = gl::Query::create(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    setGeneratorProperties( vec3(0.2f, 0.2f, 0.2f), // Where the particles are generated
                           vec3(-0.05f, -0.05f, -0.05f), // Minimal velocity
                           vec3(0.05f, 0.05f, 0.05f), // Maximal velocity
                           3.5f, // Minimum lifetime in seconds
                           8.0f, // Maximum lifetime in seconds
                           0.005f, // Rendered size
                           0.01f, // Spawn every 0.01 seconds
                           8); // Particles spawn
}


void TF_Particles::loadTexture()
{
    gl::Texture::Format mTextureFormat;
    mTextureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap();
    mParticleTexture = gl::Texture::create( loadImage( loadAsset( "particles/particleforpremul.png" ) ), mTextureFormat );    
}


float TF_Particles::grandf(float fMin, float fAdd)
{
    float fRandom = float(rand()%(RAND_MAX))/float(RAND_MAX - 1 );
    return fMin+fAdd*fRandom;
}


void TF_Particles::setGeneratorProperties(vec3 a_vGenPosition, vec3 a_vGenVelocityMin, vec3 a_vGenVelocityMax, float a_fGenLifeMin, float a_fGenLifeMax, float a_fGenSize, float fEvery, int a_iNumToGenerate)
{
    //vGenPosition = a_vGenPosition;
    vGenVelocityMin = a_vGenVelocityMin;
    vGenVelocityRange = a_vGenVelocityMax - a_vGenVelocityMin;
    
    fGenSize = a_fGenSize;
    
    fGenLifeMin = a_fGenLifeMin;
    fGenLifeRange = a_fGenLifeMax - a_fGenLifeMin;
    
    fNextGenerationTime = fEvery;
    //fElapsedTime = 0.8f;
    
    iNumToGenerate = a_iNumToGenerate;
}


void TF_Particles::loadShaders()
{
    try {
        ci::gl::GlslProg::Format mUpdateParticleGlslFormat;
        // Notice that we don't offer a fragment shader. We don't need
        // one because we're not trying to write pixels while updating
        // the position, velocity, etc. data to the screen.
        mUpdateParticleGlslFormat.vertex( loadAsset( "particles/updateSmoke.vert" ) )
        .geometry( loadAsset( "particles/updateSmoke.geom" ) )
        // This option will be either GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS,
        // depending on the structure of our data, below. We're using multiple
        // buffers. Therefore, we're using GL_SEPERATE_ATTRIBS
        .feedbackFormat( GL_INTERLEAVED_ATTRIBS )
        // Pass the feedbackVaryings to glsl
        //.feedbackVaryings( transformFeedbackVaryings );
        .feedbackVaryings({ "vPositionOut", "fLifeTimeOut", "vVelocityOut", "fSizeOut", "fAgeOut", "iTypeOut"})
        .attribLocation( "vPosition", 0 )
        .attribLocation( "fLifeTime", 1 )
        .attribLocation( "vVelocity", 2 )
        .attribLocation( "fSize", 3 )
        .attribLocation( "fAge", 4 )
        .attribLocation( "iType", 5 );
        mPUpdateGlsl = ci::gl::GlslProg::create( mUpdateParticleGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
    }
    
    try {
        ci::gl::GlslProg::Format mRenderParticleGlslFormat;
        // This being the render glsl, we provide a fragment shader.
        mRenderParticleGlslFormat.vertex( loadAsset( "particles/renderSmoke.vert" ) )
        .geometry( loadAsset( "particles/renderSmoke.geom" ) )
        .fragment( loadAsset( "particles/renderSmoke.frag" ) )
        .attribLocation("vPosition", 0 )
        .attribLocation( "fLifeTime", 1 )
        .attribLocation( "fSize", 3 )
        .attribLocation( "fAge", 4 )
        .attribLocation( "iType", 5 );
        
        
        mPRenderGlsl = ci::gl::GlslProg::create( mRenderParticleGlslFormat );
        
        mPRenderGlsl->uniform( "ParticleTex", 0 );
        mPRenderGlsl->uniform( "ObjectDepthTex", 1 );
        
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE RENDER GLSL ERROR: " << ex.what() << std::endl;
    }
}


void TF_Particles::loadBuffers()
{
    // with layout
    vector<Particle> particles;
    particles.assign( MAX_PARTICLES, Particle() );
    
    particles[0].iType = 0;
    particles[0].vPosition = vec3(0.0f, 0.0, 0.0f);
    particles[0].vVelocity = vec3(0.0f, 0.0f, 0.0f);
    particles[0].fLifeTime = 0.0f;
    particles[0].fAge = 0.0f;
    particles[0].fSize = 1.0f;
    
    // Create particle buffers on GPU and copy data into the first buffer.
    // Mark as static since we only write from the CPU once.
    mParticleBuffer[0] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW );
    mParticleBuffer[1] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW );
    
    for( int i = 0; i < 2; ++i )
    {	// Describe the particle layout for OpenGL.
        mAttributes[i] = gl::Vao::create();
        gl::ScopedVao vao( mAttributes[i] );
        
        // Define attributes as offsets into the bound particle buffer
        gl::ScopedBuffer buffer( mParticleBuffer[i] );
        gl::enableVertexAttribArray( 0 );
        gl::enableVertexAttribArray( 1 );
        gl::enableVertexAttribArray( 2 );
        gl::enableVertexAttribArray( 3 );
        gl::enableVertexAttribArray( 4 );
        gl::enableVertexAttribArray( 5 );
        gl::vertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)0); // position
        gl::vertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)12); // velocity
        gl::vertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)16); // lifetime
        gl::vertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)28); // Size
        gl::vertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)32); // Size
        gl::vertexAttribPointer(5,1,GL_INT,GL_FALSE,sizeof(Particle),(const GLvoid*)36); // type
    }
}


void TF_Particles::update( float counter )
{
    float fTimePassed = counter - fLastFrameTime;
    
    fElapsedTime += fTimePassed;
    
    gl::ScopedGlslProg	glslScope( mPUpdateGlsl );
    mPUpdateGlsl->uniform("fTimePassed", fTimePassed);
    mPUpdateGlsl->uniform("vGenVelocityRange", vGenVelocityRange);
    mPUpdateGlsl->uniform("fGenLifeMin", fGenLifeMin);
    mPUpdateGlsl->uniform("fGenLifeRange", fGenLifeRange);
    mPUpdateGlsl->uniform("fGenSize", fGenSize);
    mPUpdateGlsl->uniform( "Time", (float)counter / 60.0f );
    mPUpdateGlsl->uniform( "noise_strength", mNoise_strength );
    mPUpdateGlsl->uniform( "rotational_strength", rotational_strength );
    mPUpdateGlsl->uniform( "rotational_speed_strength", rotational_speed_strength );
    mPUpdateGlsl->uniform( "rotational_speed_strength2", rotational_speed_strength2 );
    mPUpdateGlsl->uniform( "length_scale", mLength_scale );
    mPUpdateGlsl->uniform( "noisescale", mNoiseScale );
    mPUpdateGlsl->uniform( "progression_rate", progression_rate );
    mPUpdateGlsl->uniform( "velocityDamping", velocityDamping );
    mPUpdateGlsl->uniform( "centripedAttraction", centripedAttraction );
    mPUpdateGlsl->uniform( "radiusInSphereSpread", 0.3f );
    
    if (m_isFirst) {
        mPUpdateGlsl->uniform("iNumToGenerate",			0);
    }
    else{
        mPUpdateGlsl->uniform("iNumToGenerate", iNumToGenerate);
        fElapsedTime -= fNextGenerationTime;
        vec3 vRandomSeed = vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f));
        mPUpdateGlsl->uniform("vRandomSeed", vRandomSeed);
    }
    
    
    fLastFrameTime = counter;
    
    // Bind the source data (Attributes refer to specific buffers).
    gl::ScopedVao       source( mAttributes[iCurReadBuffer] );
    gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
    
    // Bind destination as buffer base.
    gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffer[1-iCurReadBuffer] );
    
    // We begin Transform Feedback, using the same primitive that
    // we're "drawing". Using points for the particle system.
    mQuery->begin();
    gl::beginTransformFeedback( GL_POINTS );
    if (m_isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);
        m_isFirst = false;
    }
    else {
        gl::drawArrays(GL_POINTS, 0, iNumParticles);
    }
    
    gl::endTransformFeedback();
    mQuery->end();
    iNumParticles = mQuery->getValueUInt();
    
    iCurReadBuffer = 1-iCurReadBuffer;
    
    
    vGenPosition = vec3(0.0f, 2.0f*cos(counter*0.4f), 2.0f*sin(counter*0.3f));
}


void TF_Particles::draw(const CameraPersp &camera, float luminosity, float bloomLuminosity, vec2 invViewportSize, const gl::TextureRef objetcDepthTexture )
{
    gl::enableDepthWrite( false );
    //gl::enableAdditiveBlending();
    
    gl::ScopedVao                   vaoScope( mAttributes[iCurReadBuffer] );
    gl::ScopedGlslProg              glslScope( mPRenderGlsl );
    const gl::ScopedTextureBind     texScope0( mParticleTexture, 0 );
    const gl::ScopedTextureBind     texScope2( objetcDepthTexture, 1 );
    gl::ScopedState                 stateScope( GL_PROGRAM_POINT_SIZE, true );
    mPRenderGlsl->uniform( "Luminosity", luminosity);
    mPRenderGlsl->uniform( "BloomLuminosity", bloomLuminosity );
    mPRenderGlsl->uniform( "zNear", camera.getNearClip());
    mPRenderGlsl->uniform( "zFar", camera.getFarClip());
    mPRenderGlsl->uniform( "InvViewportSize", invViewportSize);

    gl::setDefaultShaderVars();
    gl::drawArrays( GL_POINTS, 0, iNumParticles );

}

