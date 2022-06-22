#include "ReactiveCam.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "glm/gtx/vector_angle.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace glm;


ReactiveCam::ReactiveCam()
{
    mAxisX                          = 0.0f;
    mAxisY                          = 0.0f;
    mAxisZ                          = 0.0f;
    mDist                           = 0.8f;
    mManuZoom = mAutoZoom           = 0.0f;
    mAutoZoom                       = false;
    mCamParams.mIsBeatRotation      = true;
    mCamParams.mIsAutoZoom          = false;
    mCamParams.mAxisXRotationSpeed  = 0.5f;
    mCamParams.mAxisYRotationSpeed  = 0.5f;
    mCamParams.mRotationSpeedCoef   = 0.05f;
    //mCamParams.mRotationSpeed       = 0.02f; // 30fps
    mCamParams.mRotationSpeed       = 0.015f;  // 60fps
    mbeatCounter                    = 0.0f;
    //mCamParams.mAutoZoomCoef        = 0.01f; // 60fps
    mCamParams.mAutoZoomCoef        = 0.02f; // 30fps
    mCamParams.mManuZoomCoef        = 0.0003f;
    mCamParams.mZoomMinDist         = 0.0f;
    mCamParams.mZoomMaxDist         = 10.0f;
    mCamParams.mRollCoef            = 0.0f;
    quat mCurrentQuat;
    quat mInitialQuat;
    mChangeRotation                 = 0;
    mInit_cam_eye                   = vec3( 1.0f, 0.0f, 0.0f );
    
    for( int i = 0; i < 6; i++ )
    {
        vKeyPressed.push_back( 0 );
    }
}


void ReactiveCam::set( float cameraDistance, vec3 lookAt, float min, float max )
{
    //mCam = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f );
    mCam.setPerspective( 35.0f, getWindowAspectRatio(), min, max );
    mCam.lookAt( mInit_cam_eye, lookAt, vec3( 0.0f, -1.0f, 0.0f ) );
    mDist = cameraDistance;
}


vec2 ReactiveCam:: getWorldToScreen( vec3 point, float fboSize_X, float fboSize_Y )
{
    return mCam.worldToScreen( point, fboSize_X, fboSize_Y );
}


vec3 ReactiveCam::getPosition(){
    return mCam.getEyePoint();
}


float ReactiveCam::getCameraDist()
{
    return mDist;
}


mat4 ReactiveCam::getViewMatrix(){
    return mCam.getViewMatrix();
}


CameraPersp& ReactiveCam::getCamera()
{
    return mCam;
}


CameraPersp ReactiveCam::getCameraMatrix()
{
    return mCam;
}


void ReactiveCam::resize( const app::WindowRef &window )
{
    mCam.setAspectRatio( window->getAspectRatio() );
}


void ReactiveCam::resize( const vec2 ratio )
{
    mCam.setAspectRatio( ratio.x / ratio.y );
}


void ReactiveCam::update( bool beat, vec3 rotationVector, bool ease, float rotationSpeed  )
{
    vec3 normal = vec3( 1.0, 1.0, 0.0 );
    float rotation = 0.0f;
    
    if ( mCamParams.mAxisXRotationSpeed == 0.0f ) mCamParams.mAxisXRotationSpeed = 0.1f;
    if ( mCamParams.mAxisYRotationSpeed == 0.0f ) mCamParams.mAxisYRotationSpeed = 0.1f;

    if ( mChangeRotation == 0 ) normal = normalize( vec3( mCamParams.mAxisXRotationSpeed, mCamParams.mAxisYRotationSpeed, 0.0f ));
    else if ( mChangeRotation == 1 ) normal = normalize( vec3( 0.0f, mCamParams.mAxisXRotationSpeed, mCamParams.mAxisYRotationSpeed ));
    else if ( mChangeRotation == 2 ) normal = normalize( vec3( mCamParams.mAxisXRotationSpeed, 0.0f, mCamParams.mAxisYRotationSpeed ));
    
    if ( mCamParams.mIsBeatRotation && beat == true )
    {
        /*app::console() << "beat " << mCamParams.mIsBeatRotation << std::endl;
        app::console() << "normal " << normal << std::endl;
        app::console() << "rotationVector " << rotationVector << std::endl;*/

        if ( mChangeRotation >= 2 ) mChangeRotation = 0;
        else mChangeRotation += 1;
        
        mbeatCounter = 0.0f;
    }
    
    if ( ease == true )
    {
        float ease = easeOutExpo( clamp( mbeatCounter, 0.0f, 1.0f ) );
        rotation = rotationSpeed * ease + ( 1.0 - ease) * mCamParams.mRotationSpeedCoef;
    }

    if ( mbeatCounter < 1.0f ) mbeatCounter += 0.015;
    
    quat incQuat = glm::angleAxis( mCamParams.mRotationSpeed + rotation, normal );

    mCurrentQuat *= incQuat;
    normalize( mCurrentQuat );
    
    vec3 cam_target = vec3();
    vec3 cam_eye = normalize( mCurrentQuat * mInit_cam_eye ) * mDist;
    vec3 cam_up = mCurrentQuat * vec3( 0.0f, -1.0f, 0.0f );;

    if ( mCamParams.mRollCoef >= 0.0f){
        cam_up = mCurrentQuat * vec3( 0.0f, sin(mAxisX*mCamParams.mRollCoef), cos(mAxisY*mCamParams.mRollCoef) );
        mCam.setWorldUp( cam_up );
    }
    
    mCam.setEyePoint(cam_eye);
    mCam.lookAt( cam_target );
    
    mAxisX += 0.01f;
    mAxisY += 0.01f;
    mAxisZ += 0.01f;
    
    if( mCamParams.mIsAutoZoom )
    {
        mManuZoom = 0.0f;
        
        if ( mDist >= mCamParams.mZoomMaxDist )
        {
            mAutoZoom = -1;
        }
        else if ( mDist <= mCamParams.mZoomMinDist )
        {
            mAutoZoom = 1;
        }
        mDist += mAutoZoom * mCamParams.mAutoZoomCoef;
    }
    else
    {
        mCamParams.mZoomMinDist = mDist;
        mDist += mManuZoom * mCamParams.mManuZoomCoef;
    }
    
    mCamParams.mDist = mDist;
    
}


void ReactiveCam::setMabuZoom( float value )
{
    mManuZoom = value;
}


void ReactiveCam::setCameraDist( float value )
{
    mDist = value;
}


void ReactiveCam::pitchYawRoll()
{

    // Not working well
    // Obtain pitch axis.
    float rotationAmount  = app::getElapsedSeconds() * mCamParams.mRollCoef;
    float pitchYawAmount  = sin(app::getElapsedSeconds()) * mCamParams.mRollCoef;

    /*vec3 pitchAxis, yawAxis;
    mCam.getBillboardVectors( &pitchAxis, &yawAxis );
    vec3 rollAxis = glm::cross(pitchAxis, yawAxis);*/

    // Obtain current orientation.
    auto orientation = mCam.getOrientation();
    // Roll
    quat roll = orientation * glm::angleAxis( rotationAmount, vec3 ( 0.0f, 0.0f, 1.0f ) );
    // pitch
    quat pitch = orientation * glm::angleAxis( pitchYawAmount, vec3 ( 1.0f, 0.0f, 0.0f ) );
    // Roll
    quat Yaw = orientation * glm::angleAxis( pitchYawAmount, vec3 ( 0.0f, 1.0f, 0.0f ) );
    
    // With incrementation, not used
    //mCurrentQuat *= roll;

    // Modify current orientation by applying pitch quaternion.
    mCam.setOrientation( pitch ); // might also be pitch * orientation
}


void ReactiveCam::setKeyEvent( std::string& keyPressed )
{
    if ( keyPressed == "0" && !mCamParams.mIsBeatRotation ) // 1 axis rotations
    {
        if ( mChangeRotation < 3 ) mChangeRotation = 3;
        else if ( mChangeRotation >= 5 ) mChangeRotation = 3;
        else mChangeRotation += 1;
    }
    
    if ( keyPressed == "1" ) // go away
    {
        if ( mManuZoom <= 1.0f ) mManuZoom += 1.0f;
    }
    
    if ( keyPressed == "2" ) // comme back
    {
        if ( mManuZoom >= -1.0f ) mManuZoom -= 1.0f;
    }
    
    if ( keyPressed == "3" )
    {
        mManuZoom = 0.0f;
    }
    
    if( keyPressed == "b" && !mCamParams.mIsBeatRotation) // 2 axis rotations manual
    {
        if ( mChangeRotation >= 3 ) mChangeRotation = 0;
        else mChangeRotation += 1;
    }
    
    if( keyPressed == "r" ) //reset
    {
        // identity quat
        mCurrentQuat = mInitialQuat;
    }
}

