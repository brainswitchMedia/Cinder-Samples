#version 330 core

// Dithering source
// https://www.shadertoy.com/view/wl3XWs


uniform sampler2D	uSampler;
uniform sampler2D   uTexBlueNoise;
uniform float       uTime;

// inputs passed from the vertex shader
in vec2             TexCoord;


layout (location = 0) out vec4 oColor;


float GetBlueNoiseDither( float grayscale, ivec2 pixelCoord, bool noise )
{
    float blueNoiseValue = texture( uTexBlueNoise, vec2( TexCoord.s, TexCoord.t ) ).r;
    
    //if ( noise == true ) blueNoiseValue = sin( blueNoiseValue * 2.0 * 3.141592 + uTime ) * 0.5 + 0.5;

    // Version 1
    return blueNoiseValue < grayscale ? 1.0 : 0.0;
    
    // Version 2
    //return step( blueNoiseValue, grayscale );
}

void main( void )
{
    float text = texture( uSampler, TexCoord.st ).r;
    float ditherColor = GetBlueNoiseDither( text, ivec2( TexCoord.st ), true );

	oColor = vec4( ditherColor, ditherColor, ditherColor, 1.0 );

}
