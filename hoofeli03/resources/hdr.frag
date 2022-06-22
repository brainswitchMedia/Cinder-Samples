#version 330 core

uniform sampler2D   uHdrBuffer1;
uniform sampler2D   uHdrBuffer2;
uniform bool        uIsBloom;
uniform bool        uIsGodrays;
uniform float       uExposure;
uniform bool        uIsgamma;
uniform float       uGamma;

in vec2             TexCoord;

layout (location = 0) out vec4 oFragColor0;


void main()
{             
    //Default standart value for uGamma = 2.2;
    
    vec3 result = vec3( 0.0, 0.0, 0.0 );

    vec3 hdrColor1 = texture( uHdrBuffer1, TexCoord.st ).rgb;
    vec3 hdrColor2 = texture( uHdrBuffer2, TexCoord.st ).rgb;
    
    if ( uIsBloom == true || uIsGodrays == true ) hdrColor1 += hdrColor2;

    // Test float from GL_RGBA32F_ARB GL_RGBA16F_ARB with clamp
    /*if ( hdrColor.r > 1.1 || hdrColor.g > 1.1 || hdrColor.b > 1.1 )
    {
        gl_FragColor = vec4( vec3(1.0, 0.0, 0.0), 1.0 );
    }
    else gl_FragColor = vec4( hdrColor, 1.0 );*/
    
    // reinhard
    result = hdrColor1 / ( hdrColor1 + vec3( 1.0 ) );
    
    //exposure
    if ( bloom == true || godrays == true ) result = vec3( 1.0 ) - exp( -hdrColor1 * uExposure );
    else result = hdrColor1;

    // also gamma correct while we're at it
    if ( uIsgamma )
    {
        result = pow( result, vec3( 1.0 / uGamma ) );
    }

    oFragColor0 = vec4( result, 1.0 );
}
