#pragma once
#include "cinder/gl/gl.h"
#include "XMLcom.h"

using namespace ci::app;


class GlobalCom {
public:
	
	XMLcom	mXMLcom;
	
	GlobalCom();
	void init( bool comType, const std::string &xmlInitPath );
    void start( bool comType, const std::string &xmlUpdatePath );
	void update();
    
    bool    IsLoopingXML;
    int     mIsBeat1;
    int     mIsBeat2;
    float   mEnergy1;
    float   mEnergy2;
	
private:
	bool mCom;
};

