#include "XMLParser.h"


XMLParser::XMLParser(): mDocXml()
{
    mLooping = false;
    mNodelength = 0;
	mIterator = 1;
}


// File must be where the executable is ( by default in build/debug )
int XMLParser::initParsingRecordedData( const std::string &xmlRecordedDataPath )
{
    mLooping = false;
    mNodelength = 1;
	mIterator = 1;
    
    fs::path updatePath( xmlRecordedDataPath );
    
    if( fs::exists( updatePath ) )
    {
        std::stringstream ss;
        XmlTree xml1( loadFile( xmlRecordedDataPath ) );
        mDocXml = xml1;
        
        console() << "start reading update xml" << std::endl;
        
        for( XmlTree::Iter it = mDocXml.begin(); it != mDocXml.end(); ++it )
        {
            if ( fromString<int>( it->getTag()) == mNodelength ) // on cherche la valeur de mIterator dans la liste du premier niveau des nodes dans le fichier xml
            {
                mNodelength++;
            }
        }
        mNodelength--;
        console() << "mNodelength: " << mNodelength << std::endl;
    }
    else std::cout << "Loader update xml file error " << std::endl;
    
    return mNodelength;
}


void XMLParser::parseRecordedData( std::vector<vec3> &conePosition, std::vector<int> &coneColor )
{
    if( mIterator < mNodelength+1 )
    {
        for( XmlTree::Iter it = mDocXml.begin(); it != mDocXml.end(); ++it )
        {
                XmlTree xmlNode = mDocXml.getChild( toString( mIterator ) ); // puis on copie le node
                
                for( XmlTree::Iter child = xmlNode.begin(); child != xmlNode.end(); ++child )
                {
                    
                    if ( fromString<int>( child->getTag() ) < 5  )
                    {
                        switch ( fromString<int>( child->getTag() ) )
                        {
                            case 1:{
                                mPosition.x =  fromString<double>( child->getValue() );
                                app::console() << "x " << fromString<double>( child->getValue() ) << std::endl;
                                break;
                            }
                            case 2:{
                                mPosition.y =  fromString<double>( child->getValue() );
                                app::console() << "y " << fromString<double>( child->getValue() ) << std::endl;
                                break;
                            }
                            case 3:{
                                mPosition.z =  fromString<double>( child->getValue() );
                                app::console() << "z " << fromString<double>( child->getValue() ) << std::endl;
                                break;
                            }
                            case 4:{
                                mColor =  fromString<int>( child->getValue() );
                                app::console() << "color " << fromString<int>( child->getValue() ) << std::endl;
                                break;
                            }
                            default:
                                break;
                        }
                    }
            }
            conePosition[ mIterator - 1 ] = mPosition;
            coneColor[ mIterator - 1 ] = mColor;
            app::console() << "mPosition " << mPosition << std::endl;
            //cinder::app::console() <<  "mConePositions[mIter]" << mConePositions[i] << " mIter:" << i << std::endl;

            mIterator++;
        }

        mLooping = true;
    }
    else
    {
        mLooping = false;
    }
}


bool XMLParser::isLooping()
{
    return mLooping;
}



