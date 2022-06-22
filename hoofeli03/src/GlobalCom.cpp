#include "GlobalCom.h"

GlobalCom::GlobalCom()
{
}


void GlobalCom::init( bool comType, const std::string &xmlInitPath )
{
	mCom = comType;	// 0 for osccom, 1 for xmlcom
	
    IsLoopingXML = false;
    
	if ( mCom )
    {
        mXMLcom.init( xmlInitPath );
    }
}

void GlobalCom::start( bool comType, const std::string &xmlUpdatePath )
{
	mCom = comType;	// 0 for osccom, 1 for xmlcom
	
    IsLoopingXML = false;
    
	if ( mCom )
    {
        mXMLcom.start( xmlUpdatePath );
    }
    
	//else mOSCcom.init();
}


void GlobalCom::update()
{
	if ( mCom ) 
	{
		mXMLcom.update();
        IsLoopingXML = mXMLcom.isLooping();
        mIsBeat1 = mXMLcom.mIsBeat1;
        mIsBeat2 = mXMLcom.mIsBeat2;
        mEnergy1 = mXMLcom.mEnergy1;
        mEnergy2 = mXMLcom.mEnergy2;

	}
	else
	{
		//mOSCcom.update();
	}	
}

