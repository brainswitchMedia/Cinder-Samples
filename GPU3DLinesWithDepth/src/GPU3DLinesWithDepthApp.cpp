/*
 Copyright (c) 2021, brainswitchMedia All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Draw Perfect Lines in 3D with depth information
 
 This code is heavily based on Paul's GeometryShader Sample:
 => https://github.com/paulhoux/Cinder-Samples/tree/master/GeometryShader
 
 Depth calculation  is based on opengl depthbuffer documentation:
 https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
 
 and Sergejs Kovrovs's article:
 https://gist.github.com/kovrov/a26227aeadde77b78092b8a962bd1a91
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 
 Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/ImageIo.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GPU3DLinesWithDepthApp : public App {
public:
    static void prepare( Settings *settings );
    
    void setup() override;
    void update() override;
    void draw() override;
    void keyDown( KeyEvent event ) override;
    void resize() override;
    
protected:
    void loadLineShader( const std::string &path );
    void    createBatches();
    
protected:
    float                   mRadius;
    float                   mThickness;
    float                   mLimit;
    
    int                     mDepthCalculationMode;
    float                   mObjectDrawingSizeCoef;
    
    vec4                    mDefaultEdgeColor;
    std::vector<vec3>       mPoints;
    gl::GlslProgRef         mShader, mRenderObjectGlsl;
    gl::VboMeshRef          mVboMesh;
    gl::BatchRef            mBatchObjects, mBatchToScreenRect;
    ci::gl::FboRef          mObjectsFBO, mLineFBO;
    ci::gl::Texture2dRef    mObjectsRenderFBO[ 2 ];
    ci::gl::Texture2dRef    mLineRenderFBO[ 2 ];
    
    CameraPersp mCam;
};


void GPU3DLinesWithDepthApp::prepare( Settings *settings )
{
    settings->setTitle( "Drawing smooth lines using a geometry shader" );
    settings->setWindowSize( 640, 640 );
    settings->setHighDensityDisplayEnabled();
}


void GPU3DLinesWithDepthApp::setup()
{
    mRadius = 5.0f;
    mThickness = 4.0f;
    mLimit = 0.75f;
    mDepthCalculationMode = 1;
    mObjectDrawingSizeCoef = 0.99f;
    mDefaultEdgeColor = vec4( 0.6f, 0.6f, 0.6f, 1.0f );

    mPoints.clear();
    
    loadLineShader( "lineshaders/lines1.geom" );
    
    createBatches();
    
    mCam = CameraPersp( 606, 400, 60.0f, 0.1f, 10.0f );
    mCam.lookAt( vec3( 5, 0, 5 ), vec3( 0 ) );
    
}


void GPU3DLinesWithDepthApp::update()
{
    mPoints.clear();
    mVboMesh.reset();
    
    for( auto i = 0; i < 20000; ++i ) {
        mPoints.push_back( 0.5f*vec3( sin(0.00002f*i*getElapsedSeconds())*3.0f*(float)cos(3.1415f*i*0.005f), sin(0.00002f*i*getElapsedSeconds())*3.0f*(float)sin(3.1415f*i*0.005f) , 5.0f - i*0.0005f ) );
    }
    
    if( mPoints.size() > 1 ) {
        // create a new vector that can contain 3D vertices
        std::vector<vec3> vertices;
        
        // to improve performance, make room for the vertices + 2 adjacency vertices
        vertices.reserve( mPoints.size() + 2 );
        
        // first, add an adjacency vertex at the beginning
        vertices.push_back( 2.0f * (mPoints[0] - mPoints[1]) );
        
        for( std::vector<vec3>::iterator itr = mPoints.begin(); itr != mPoints.end(); ++itr )
        vertices.push_back( *itr );
        
        // next, add an adjacency vertex at the end
        size_t n = mPoints.size();
        vertices.push_back( 2.0f * (mPoints[n - 1] - mPoints[n - 2]) );
        
        // now that we have a list of vertices, create the index buffer
        n = vertices.size() - 2;
        std::vector<uint16_t> indices;
        indices.reserve( n * 4 );
        
        for( auto i = 1; i < vertices.size() - 2; ++i ) {
            indices.push_back( i - 1 );
            indices.push_back( i );
            indices.push_back( i + 1 );
            indices.push_back( i + 2 );
        }
        
        // finally, create the mesh
        gl::VboMesh::Layout layout;
        layout.usage( GL_STATIC_DRAW ).attrib( geom::POSITION, 3 );
        layout.usage( GL_STATIC_DRAW ).attrib( geom::COLOR, 4 );
        
        std::vector<vec4> edgeColors( vertices.size(), mDefaultEdgeColor );
        
        mVboMesh = gl::VboMesh::create( vertices.size(), GL_LINES_ADJACENCY_EXT, { layout }, indices.size() );
        mVboMesh->bufferAttrib( geom::POSITION, vertices.size() * sizeof( vec3 ), vertices.data() );
        mVboMesh->bufferAttrib( geom::COLOR, edgeColors );
        mVboMesh->bufferIndices( indices.size() * sizeof( uint16_t ), indices.data() );
    }
    
    mCam.lookAt( vec3( 5.0f * cos( getElapsedSeconds() * 0.7f ), 0, 5.0f * sin( getElapsedSeconds() * 0.7f ) ), vec3( 0 ) );
}


void GPU3DLinesWithDepthApp::draw()
{
    // Render Objects color and Depth
    {
        const gl::ScopedFramebuffer scopedFrameBuffer( mObjectsFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mObjectsFBO->getSize() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        gl::clear();
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatrices( mCam );
        gl::enableDepthWrite();
        gl::enableDepthRead();
        mBatchObjects->getGlslProg()->uniform( "radius", 1.0f );
        mBatchObjects->draw();
        
        // Render lines
        if( mDepthCalculationMode != 3 && mShader && mVboMesh)
        {
            gl::ScopedGlslProg shader( mShader );
            mShader->uniform( "uSamplerObjectsDepth", 0 );
            mShader->uniform( "WIN_SCALE", vec2( getWindowSize() ) ); // casting to vec2 is mandatory!
            mShader->uniform( "MITER_LIMIT", mLimit );
            mShader->uniform( "THICKNESS", mThickness );
            mShader->uniform( "depthMode", mDepthCalculationMode );
            mShader->uniform( "zFar", mCam.getFarClip());
            mShader->uniform( "zNear", mCam.getNearClip());
            gl::draw( mVboMesh );
        }
    }
    
    
    if ( mDepthCalculationMode == 3 )
    {
        // Render lines
        {
            const gl::ScopedFramebuffer scopedFrameBuffer( mLineFBO );
            const gl::ScopedViewport scopedViewport( ivec2( 0 ), mLineFBO->getSize() );
            const static GLenum buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
            };
        
            gl::drawBuffers( 2, buffers );
            gl::clear();
            // Set color to white for the depth attachment
            static const float white[] = { 1, 1, 1, 1 };
            glClearBufferfv(GL_COLOR, 1, white);
        
            const gl::ScopedMatrices scopedMatrices;
            gl::setMatrices( mCam );
            gl::enableDepthWrite();
            gl::enableDepthRead();
        
            if( mShader && mVboMesh )
            {
                const gl::ScopedTextureBind scopedTextureBind0( mObjectsFBO->getDepthTexture(), 0 );
                gl::ScopedGlslProg shader( mShader );
                mShader->uniform( "uSamplerObjectsDepth", 0 );
                mShader->uniform( "WIN_SCALE", vec2( getWindowSize() ) ); // casting to vec2 is mandatory!
                mShader->uniform( "MITER_LIMIT", mLimit );
                mShader->uniform( "THICKNESS", mThickness );
                mShader->uniform( "zFar", mCam.getFarClip());
                mShader->uniform( "zNear", mCam.getNearClip());
                mShader->uniform( "depthMode", mDepthCalculationMode );
                gl::draw( mVboMesh );
            }
        }
        
        // Render Object color
        {
            const gl::ScopedFramebuffer scopedFrameBuffer( mObjectsFBO );
            const gl::ScopedViewport scopedViewport( ivec2( 0 ), mObjectsFBO->getSize() );
            gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
            gl::clear();
            const gl::ScopedMatrices scopedMatrices;
            gl::setMatrices( mCam );
            gl::enableDepthWrite();
            gl::enableDepthRead();
            mBatchObjects->getGlslProg()->uniform( "radius", mObjectDrawingSizeCoef );
            mBatchObjects->draw();
        }
    }
    
    // Render our final image to the screen
    {
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), toPixels( getWindowSize() ) );
        const gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( toPixels( getWindowSize() ) );
        gl::translate( toPixels( getWindowSize() / 2 )  );
        gl::scale( toPixels( getWindowSize() ) );
        gl::clear();
        gl::disableDepthRead();
        gl::disableDepthWrite();
        
        if ( mDepthCalculationMode == 3 ) mObjectsRenderFBO[1]->bind(0);
        else mObjectsRenderFBO[0]->bind(0);
        
        const gl::ScopedTextureBind scopedTextureBind1( mLineRenderFBO[0], 1 );
        const gl::ScopedTextureBind scopedTextureBind2( mObjectsFBO->getDepthTexture(), 2 );
        const gl::ScopedTextureBind scopedTextureBind3( mLineRenderFBO[1], 3 );
        mBatchToScreenRect->getGlslProg()->uniform( "zFar", mCam.getFarClip());
        mBatchToScreenRect->getGlslProg()->uniform( "zNear", mCam.getNearClip());
        mBatchToScreenRect->getGlslProg()->uniform( "depthMode", mDepthCalculationMode );
        mBatchToScreenRect->draw();
        
        if ( mDepthCalculationMode == 3 ) mObjectsRenderFBO[1]->unbind(0);
        else mObjectsRenderFBO[0]->unbind(0);
    }
}


void GPU3DLinesWithDepthApp::keyDown( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
        quit();
        break;
        case KeyEvent::KEY_SPACE:
        mPoints.clear();
        // invalidate mesh
        mVboMesh.reset();
        break;
        case KeyEvent::KEY_m:
        if( mThickness > 1.0f )
        mThickness -= 1.0f;
        break;
        case KeyEvent::KEY_n:
        if( mThickness < 100.0f )
        mThickness += 1.0f;
        break;
        case KeyEvent::KEY_EQUALS: // For Macs without a keypad or a plus key
        if( !event.isShiftDown() ) {
            break;
        }
        case KeyEvent::KEY_PLUS:
        case KeyEvent::KEY_KP_PLUS:
        if( mLimit < 1.0f )
        mLimit += 0.1f;
        break;
        case KeyEvent::KEY_MINUS:
        case KeyEvent::KEY_KP_MINUS:
        if( mLimit > -1.0f )
        mLimit -= 0.1f;
        break;
        case KeyEvent::KEY_1:
        mDepthCalculationMode = 1;
        break;
        case KeyEvent::KEY_2:
        mDepthCalculationMode = 2;
        break;
        case KeyEvent::KEY_3:
        mDepthCalculationMode = 3;
        break;
        case KeyEvent::KEY_p:
        if( mObjectDrawingSizeCoef < 1.0f )
        mObjectDrawingSizeCoef += 0.01f;
        break;
        case KeyEvent::KEY_o:
        if( mObjectDrawingSizeCoef > 0.01f )
        mObjectDrawingSizeCoef -= 0.01f;
        break;

        case KeyEvent::KEY_F7:
        loadLineShader( "lineshaders/lines1.geom" );
        break;
        case KeyEvent::KEY_F8:
        loadLineShader( "lineshaders/lines2.geom" );
        break;
        default:
        break;
    }
}


void GPU3DLinesWithDepthApp::resize()
{
    mCam.setAspectRatio( getWindowAspectRatio() );
    
    // Choose window size based on selected quality
    ivec2 winSize	= toPixels( getWindowSize() );
    int32_t h		= winSize.y;
    int32_t w		= winSize.x;
    
    console() << "window: " << toPixels( getWindowSize() ) << std::endl;
    
    // Texture format for depth buffers
    gl::Texture2d::Format depthTextureFormat = gl::Texture2d::Format()
    .internalFormat( GL_DEPTH_COMPONENT32F )
    .magFilter( GL_LINEAR )
    .minFilter( GL_LINEAR )
    .wrap( GL_CLAMP_TO_EDGE )
    .dataType( GL_FLOAT );
    
    // Set up the buffer for rendering object and depth
    {
        const ivec2 sz = ivec2( w, h );
        mObjectsRenderFBO[ 0 ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                       .internalFormat( GL_RGB32F_ARB )
                                                       .magFilter( GL_LINEAR )
                                                       .minFilter( GL_LINEAR )
                                                       .wrap( GL_CLAMP_TO_EDGE )
                                                       .dataType( GL_FLOAT ));
        mObjectsRenderFBO[ 1 ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                       .internalFormat( GL_RGB32F_ARB )
                                                       .magFilter( GL_LINEAR )
                                                       .minFilter( GL_LINEAR )
                                                       .wrap( GL_CLAMP_TO_EDGE )
                                                       .dataType( GL_FLOAT ));
        gl::Fbo::Format fboFormat;
        fboFormat.depthTexture( depthTextureFormat );
        fboFormat.attachment( GL_COLOR_ATTACHMENT0, mObjectsRenderFBO[ 0 ] );
        fboFormat.attachment( GL_COLOR_ATTACHMENT1, mObjectsRenderFBO[ 1 ] );
        
        mObjectsFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mObjectsFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mObjectsFBO->getSize() );
        gl::clear();
    }
    
    // Set up the buffer for rendering lines
    {
        const ivec2 sz = ivec2( w, h );
        mLineRenderFBO[ 0 ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                    .internalFormat( GL_RGB32F_ARB )
                                                    .magFilter( GL_LINEAR )
                                                    .minFilter( GL_LINEAR )
                                                    .wrap( GL_CLAMP_TO_EDGE )
                                                    .dataType( GL_FLOAT ));
        mLineRenderFBO[ 1 ] = gl::Texture2d::create( sz.x, sz.y, gl::Texture2d::Format()
                                                    .internalFormat( GL_RGB32F_ARB )
                                                    .magFilter( GL_LINEAR )
                                                    .minFilter( GL_LINEAR )
                                                    .wrap( GL_CLAMP_TO_EDGE )
                                                    .dataType( GL_FLOAT ));
        gl::Fbo::Format fboFormat;
        fboFormat.depthTexture( depthTextureFormat );
        fboFormat.attachment( GL_COLOR_ATTACHMENT0, mLineRenderFBO[ 0 ] );
        fboFormat.attachment( GL_COLOR_ATTACHMENT1, mLineRenderFBO[ 1 ] );
        
        mLineFBO = gl::Fbo::create( sz.x, sz.y, fboFormat );
        const gl::ScopedFramebuffer scopedFramebuffer( mLineFBO );
        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mLineFBO->getSize() );
        gl::clear();
    }
    
    // invalidate mesh
    mVboMesh.reset();
    
    // force redraw
    update();
}


void GPU3DLinesWithDepthApp::loadLineShader( const std::string &path )
{
    // Load the geometry shader as a text file into memory and prepend the header
    const DataSourceRef geomFile = loadAsset( path );
    
    // Load vertex and fragments shaders as text files and compile the shader
    try {
        const DataSourceRef vertFile = loadAsset( "lineshaders/lines.vert" );
        const DataSourceRef fragFile = loadAsset( "lineshaders/lines.frag" );
        mShader = gl::GlslProg::create( vertFile, fragFile, geomFile );
    }
    catch( const std::exception &e ) {
        console() << "Could not compile shader:" << e.what() << std::endl;
    }
}


void GPU3DLinesWithDepthApp::createBatches()
{
    
    // 3D Objects
    {
        auto sphere                 = geom::Sphere().radius( 1.0f ).subdivisions( 30 );
        mRenderObjectGlsl           = gl::GlslProg::create( loadAsset( "objects.vert" ), loadAsset( "objects.frag" ) );
        mBatchObjects               = gl::Batch::create( sphere, mRenderObjectGlsl );
        gl::ScopedGlslProg glslScp( mRenderObjectGlsl );
        
    }
    
    // To screen
    gl::VboMeshRef  rect			= gl::VboMesh::create( geom::Rect() );
    gl::GlslProgRef composite       = gl::GlslProg::create( loadAsset( "pass_through.vert" ), loadAsset( "composite.frag" ) );
    mBatchToScreenRect              = gl::Batch::create( rect, composite );
    mBatchToScreenRect->getGlslProg()->uniform( "uObjectsText", 0 );
    mBatchToScreenRect->getGlslProg()->uniform( "uLineText", 1 );
    mBatchToScreenRect->getGlslProg()->uniform( "uObjectsDepthText", 2 );
    mBatchToScreenRect->getGlslProg()->uniform( "uLineDepthText", 3 );
    
}


CINDER_APP( GPU3DLinesWithDepthApp, RendererGl( RendererGl::Options().msaa( 16 ) ), &GPU3DLinesWithDepthApp::prepare )
