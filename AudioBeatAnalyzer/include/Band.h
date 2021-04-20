//
//  Band.h
//  InputAudioAnalyzer
//
//  Created by Daniel Schweitzer on 09.12.20.
//
//

#pragma once
#include <list>

using namespace ci;
using namespace ci::app;
using namespace std;


class Band {
    
public:
    Band(){};
    
    //! Easing equation for an exponential (2^t) ease-out, decelerating from zero velocity.
    inline float easeOutExpo( float t )
    {
        return t == 1 ? 1 : -math<float>::pow( 2, -10 * t ) + 1;
    }
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    inline float linearConversion( float a0, float a1, float b0, float b1, float in )
    {
        return ( ( in - a0 ) / ( a1 - a0 )) * ( b1 - b0 ) + b0;
    }

    
    bool    active;
    bool    beat;
    int     beatCounter;
    double	beatSensitivity;
    double  beatThreshold;
    double  bandEnergy;
    double  bandMagnitude;
    double  smoothBandMagnitude;
    double  bandMagnitudeCorrected;
    int     bandSize; // number of bins in the band
    std::list<double> historyEnergyBuffer;
    
};
