#pragma once
#include "cinder/Vector.h"
#include "cinder/Xml.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class XMLcom {
public:
	XMLcom();
	void init( const std::string &xmlInitPath );
    void start( const std::string &xmlUpdatePath );
	void update();
	bool isLooping();
	int getIterator();
    
    int mStep;
    int mIsBeat0;
    int mIsBeat1;
    int mIsBeat2;
    int mIsRangeBeat;
    float mEnergy0;
    float mEnergy1;
    float mEnergy2;
    float mRangeEnergy;
    float mReadTime;
    
private:
	int						mNodelength;
	int						mIterator;
    XmlTree                 nDocXml;
    bool                    mLooping;
    
};
