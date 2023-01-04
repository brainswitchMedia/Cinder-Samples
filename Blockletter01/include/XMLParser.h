#pragma once
#include "cinder/Vector.h"
#include "cinder/Xml.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;

class XMLParser {
public:
	XMLParser();
    int initParsingRecordedData( const std::string &xmlRecordedDataPath );
	void parseRecordedData( std::vector<vec3> &conePosition, std::vector<int> &coneColor );
	bool isLooping();
	
    std::vector<vec3>		mConePositions;
    vec3                    mPosition;
    int                     mColor;
    
private:
	int						mNodelength;
	int						mIterator;
    XmlTree                 mDocXml;
    bool                    mLooping;
    

};
