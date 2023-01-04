#include "File.h"


File::File()
{
    mIsStreamOpen = false;
    mIsXMLFileLoaded = false;
}


void File::openStream( const fs::path path )
{
    fs::path appPath = getAppPath();
    appPath.remove_filename();
    appPath.remove_filename();
    appPath.remove_filename();
    mFilePath = appPath / path;
    
    if( fs::exists( mFilePath ) && mIsStreamOpen == false )
    {
        // File will be create if it does not exist and will be cleared if it exist
        console() << "Open file: " << mFilePath << endl;
        mTxtStream.open( mFilePath.string() );
        mIsStreamOpen = true;
    }
    else
    {
        console() << "failed to open file. Wrong path : " << mFilePath << endl;
    }
}


void File::closeStream()
{
    if(  mIsStreamOpen == true )
    {
        mTxtStream.close();
        mIsStreamOpen = false;
        console() << "close file: " << mFilePath << endl;
    }

}


void File::writeLine( const string text )
{
    mTxtStream << text;
}


void File::loadXMLFile( const fs::path xmlInitPath )
{
    fs::path appPath = getAppPath();
    appPath.remove_filename();
    appPath.remove_filename();
    appPath.remove_filename();
    mXMLFilePath = appPath / xmlInitPath;

    if( fs::exists( mXMLFilePath ) )
    {
        XmlTree xml( loadFile( mXMLFilePath ) );
        console() << "Open XML file: " << mXMLFilePath << endl;
        mIsXMLFileLoaded = true;
    }
    else
    {
        console() << "failed to open XML file. Wrong path : " << mXMLFilePath << endl;
    }
}





