//
//  Band.h
//  InputAudioAnalyzer
//
//  Created by Daniel Schweitzer on 09.12.20.
//
//

#pragma once
#include "cinder/Xml.h"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace glm;

class File {
    
public:
    File();
    void openStream( const fs::path path );
    void closeStream();
    void loadXMLFile( const fs::path xmlInitPath );
    void writeLine( const string text );
    inline bool isTxtStreamOpen() { return mIsStreamOpen; };
    inline bool isXMLFileLoaded() { return mIsXMLFileLoaded; };
    inline uint32 getTxtIterator() { return mTxtIterator; };
    inline void setTxtIterator( uint32 value ) { mTxtIterator = value; };
    inline void incrementTxtIterator() { mTxtIterator += 1; };
    
    ofstream                mTxtStream;
    XmlTree                 mDocXml;

private:
    fs::path                mFilePath;
    fs::path                mXMLFilePath;
    bool                    mIsStreamOpen;
    bool                    mIsXMLFileLoaded;
    int						mNodelength;
    uint32                  mTxtIterator;
};
