#version 330 core

uniform sampler2D	uBackground;
uniform sampler2D	uForground;

uniform float		attenuation;

// inputs passed from the vertex shader
in vec2             TexCoord0;

layout (location = 0) out vec4 oColor;


const float	kExposure	= 6.0;
const float kLuminosity	= 0.333;
const float	kThreshold	= 0.05;
const float	kEdge		= 0.55;
const float powValue    = 2.0;

float czm_luminance(vec3 rgb)
{
    // Algorithm from Chapter 10 of Graphics Shaders.
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    return dot(rgb, W);
}


void main()
{
    vec3 colorBG = texture(uBackground, vec2(TexCoord0.x, TexCoord0.y)).rgb;
    
    // Set to black if the luminence is very low to eliminate persistance color which never disapear
    /*if ( czm_luminance( colorBG.rgb ) < 0.01 ){
        colorBG.rgb = vec3(0.0, 0.0, 0.0);
    }*/
    vec3 colorFG = texture(uForground, vec2(TexCoord0.x, TexCoord0.y)).rgb;
        
    // version 1
    //vec3 colorResult = mix(colorBG, colorFG, attenuation);
    
    // version 2
    vec3 colorResult = pow( pow( colorFG, vec3(powValue, powValue , powValue) ) * attenuation + pow( colorBG, vec3(powValue, powValue , powValue) ) * ( 1.0 - attenuation), vec3(1.0/powValue, 1.0/powValue , 1.0/powValue));

    
    oColor = vec4( colorResult, 1.0);
}
