#version 330 core

// Superior texture filter for dynamic pixel art upscaling
// https://csantosbh.wordpress.com/2014/02/05/automatically-detecting-the-texture-filter-threshold-for-pixelated-magnifications/

precision highp float;

uniform sampler2D   uSampler;
uniform vec2        uTextureSize;

in vec2             TexCoord;
in vec2             vUv;


layout (location = 0) out vec4 oColor;


void main()
{
    float w = uTextureSize.x;
    float h = uTextureSize.y;
    
    //vec2 alpha = 0.7 * vec2( dFdx( vUv.x ), dFdy( vUv.y ) );
    // between 0 and 1, 0 -> sharpest, 1-> softest
    vec2 alpha = vec2( 0.0 );

    vec2 x = fract( vUv );
    
    vec2 x_ = clamp( 0.5 / alpha * x, 0.0, 0.5 ) + clamp( 0.5 / alpha * ( x - 1.0 ) + 0.5, 0.0, 0.5 );
    
    vec2 texCoord = ( floor( vUv ) + x_ ) / vec2( w, h );
    
    oColor = vec4( texture( uSampler, texCoord ).rbg, 1.0 );
    //oColor = vec4( texture( uSampler, TexCoord ).rbg, 1.0 );

    //oColor = vec4( texCoord.x, texCoord.y, 0.0 , 1.0 );

}
