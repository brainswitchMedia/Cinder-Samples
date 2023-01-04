//
//  ReactiveCam.h
//  MyCamera3
//
//  Created by Daniel Schweitzer on 06.03.18.
//
//


#pragma once
#include "cinder/Camera.h"
#include "cinder/Vector.h"
#include "cinder/Easing.h"
#include <string.h>

using namespace ci;
using namespace std;


class ReactiveCam {
    
public:
    ReactiveCam();
    void set( float cameraDistance, vec3 lookAt, float min, float max );
    void update( bool beat, vec3 rotationVector, bool isEase, float rotationSpeed );
    void pitchYawRoll();
    void resize( const app::WindowRef &window );
    void resize( const vec2 ratio );
    vec3 getPosition();
    vec3 getViewDirection();
    float getCameraDist();
    CameraPersp& getCamera();
    CameraPersp  getCameraMatrix();
    mat4 getViewMatrix();
    vec2 getWorldToScreen( vec3 point, float fboSize_X, float fboSize_Y);
    void setKeyEvent( std::string& keyPressed );
    void setMabuZoom( float value );
    void setCameraDist( float value );

    ~ReactiveCam(){};
    
    struct Params
    {
        bool        mIsBeatRotation, mIsAutoZoom;
        float       mAxisXRotationSpeed, mAxisYRotationSpeed, mRotationSpeed;
        float       mAutoZoomCoef, mManuZoomCoef, mZoomMinDist, mZoomMaxDist;
        float       mRollCoef;
        float       mRotationSpeedCoef;
        float       mDist;
    };
    
    struct Params   mCamParams;
    
    vector<int>     vKeyPressed;
    
private:
    CameraPersp     mCam;
    float           mAxisX, mAxisY, mAxisZ;
    float           mDist, mAutoZoom, mManuZoom;
    int             mChangeRotation;
    float           mbeatCounter;
    vec3            mInit_cam_eye;
    quat            mCurrentQuat, mInitialQuat;
};
