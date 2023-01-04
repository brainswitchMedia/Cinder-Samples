#include "XMLcom.h"


XMLcom::XMLcom(): nDocXml()
{
    mLooping = false;
    mNodelength = 0;
	mIterator = 1;
}


void XMLcom::init( const std::string &xmlInitPath )
{
    mLooping = false;
    mNodelength = 0;
	mIterator = 1;
    
    // SETUP OFSTREAM
    fs::path initPath( xmlInitPath );
    if( fs::exists( initPath ) )
    {
        stringstream ss;
        XmlTree xml0( loadFile( xmlInitPath ) );
        console() << "start reading init xml" << std::endl;
        /* Nothing to init in this programm
        for( XmlTree::Iter it = xml0.begin(); it != xml0.end(); ++it )
        {
            switch ( fromString<int>( it->getTag() ) )
            {
                // Particles
                case 1:{
                    float test1 =  fromString<float>( it->getValue() );
                    break;
                }
                case 2:{
                    float test2 = fromString<float>( it->getValue() );
                    break;
                }
                default:
                    break;
            }
        }*/
    }
    else std::cout << "Loader init xml file error " << std::endl;
}


void XMLcom::start( const std::string &xmlUpdatePath )
{
    mLooping = false;
    mNodelength = 1;
	mIterator = 1;
    
    fs::path updatePath( xmlUpdatePath );
    
    if( fs::exists( updatePath ) )
    {
        stringstream ss;
        XmlTree xml1( loadFile( xmlUpdatePath ) );
        nDocXml = xml1;
        
        console() << "start reading update xml" << std::endl;
        
        for( XmlTree::Iter it = nDocXml.begin(); it != nDocXml.end(); ++it )
        {
            if ( fromString<int>( it->getTag()) == mNodelength ) // on cherche la valeur de mIterator dans la liste du premier niveau des nodes dans le fichier xml
            {
                mNodelength++;
            }
        }
        mNodelength--;
    }
    else std::cout << "Loader update xml file error " << std::endl;
}


void XMLcom::update()
{
    
    if( mIterator < mNodelength+1 )
    {
        for( XmlTree::Iter it = nDocXml.begin(); it != nDocXml.end(); ++it )
        {
            if ( fromString<int>( it->getTag()) == mIterator ) // on cherche la valeur de mIterator dans la liste du premier niveau des nodes dans le fichier xml
            {
                XmlTree xmlNode = nDocXml.getChild( toString( mIterator ) ); // puis on copie le node

                for( XmlTree::Iter child = xmlNode.begin(); child != xmlNode.end(); ++child )
                {
                    if ( fromString<int>( child->getTag() ) < 11  )
                    {
                        switch ( fromString<int>( child->getTag() ) )
                        {
                            case 1:{
                                mStep = fromString<int>( child->getValue() );
                                break;
                            }
                            case 2:{
                                mIsBeat0 = fromString<int>( child->getValue() );
                                break;
                            }
                            case 3:{
                                mEnergy0 = fromString<float>( child->getValue() );
                                break;
                            }
                            case 4:{
                                mIsBeat1 = fromString<int>( child->getValue() );
                                break;
                            }
                            case 5:{
                                mEnergy1 = fromString<float>( child->getValue() );
                                break;
                            }
                            case 6:{
                                mIsBeat2 = fromString<int>( child->getValue() );
                                break;
                            }
                            case 7:{
                                mEnergy2 = fromString<float>( child->getValue() );
                                break;
                            }
                            case 8:{
                                mIsRangeBeat = fromString<int>( child->getValue() );
                                break;
                            }
                            case 9:{
                                mRangeEnergy = fromString<float>( child->getValue() );
                                break;
                            }
                            case 10:{
                                mReadTime =  fromString<float>( child->getValue() );
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    else continue;
                }
            }
        }
        mIterator++;
        mLooping = true;
    }
    else
    {
        mLooping = false;
    }
}


bool XMLcom::isLooping()
{
    return mLooping;
}


int XMLcom::getIterator()
{
    return mIterator;
}
