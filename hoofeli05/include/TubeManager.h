#pragma once
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include <utility>
#include "cinder/Rand.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"
#include "cinder/Log.h"

#include "Resources.h"
#include "Tube.h"
#include "TubeParticles.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class TubeManager {
public:
	
	TubeManager();
	~TubeManager() {}
    void setup( const std::string &pathVert, const std::string &pathFrag, const std::string &pathTexture1, const std::string &pathTexture2 );
    void addTube( int shapeType );
    void removeTube();
    TubeParticles* getTupeParticle( int i );
    void update( vector <ci::vec3> &alocParticleEmitters );
	void drawTubes( CameraPersp *lightCameraPersp, float param_DistanceConverstion, float brightness, float counter, int texture );
    mat3 rotationAlign( vec3 d, vec3 z );
    
    struct Params
	{
        bool                mIsTubes;
        int                 mTubeNumber;
        bool                mAutoGrowDiam;
        bool                mAutoGrowLength;
        float               mMaxScale0;
        float               mMaxScale1;
        int                 mLengthParticleLoc;
        int					mTubeLength;
        float               mVelCoef;
        float               mVelDamping;
        int                 mIsStripesType;
        bool                mIsBloom;
        bool                mIsGodRays;
        
        float               mMagnitude1;
	};
    
    struct Params           mTubesParams;
       
private:
    
    // TUBE VAR //////////////////////////////////////////////////////////////////////
    ci::BSpline3f               mBSpline;
	std::vector<ci::vec3>       prof;
    float                       mLengthGrowtimer = 0.0f;
    
    bool                        mParallelTransport;
	bool                        mDrawCurve;
	bool                        mDrawFrames;
	bool                        mDrawMesh;
	bool                        mDrawSlices;
	bool                        mWireframe;
    int                         mShape;
    bool                        isTubeClosed;
    
    std::vector<Tube*>          mTubes;
    std::vector<TubeParticles*> mTubeParticles;

    int                         mNumberOfTubes;
    int                         mCurveNumSegs = 40;
    float                       mRotationValue = 0.0f;
    
    // Shaders ////////////////////////////////////////////////////////////////////////
    gl::GlslProgRef             mTubesColorsShader;
    
    // Textures ///////////////////////////////////////////////////////////////////////
    gl::TextureRef              mTexture1;
    gl::TextureRef              mTexture2;
};


