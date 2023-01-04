#version 330 core

uniform sampler2D   uHdrBuffer1;
uniform sampler2D   uHdrBuffer2;
uniform sampler2D   uHdrBuffer3;

uniform bool        uIsBloom;
uniform bool        uIsGodrays;
uniform bool        uIsgamma;
uniform float       uMixBloomGodrays;
uniform float       uChromaticAberration;
uniform float       uExposure;
uniform float       uGamma;

in vec2             TexCoord;

layout (location = 0) out vec4 oFragColor0;


void main()
{             
    vec3 result = vec3( 0.0, 0.0, 0.0 );

    vec3 trail = vec3( 0.0 );
   
    trail.r = texture( uHdrBuffer1, vec2( TexCoord.x + uChromaticAberration,	TexCoord.y ) ).r;
    trail.g = texture( uHdrBuffer1, vec2( TexCoord.x + 0.0,                   TexCoord.y ) ).g;
    trail.b = texture( uHdrBuffer1, vec2( TexCoord.x - uChromaticAberration,	TexCoord.y ) ).b;
    
    vec3 bloom = vec3( 0.0 );
    bloom.r = texture( uHdrBuffer2, vec2( TexCoord.x + uChromaticAberration,	TexCoord.y ) ).r;
    bloom.g = texture( uHdrBuffer2, vec2( TexCoord.x + 0.0,                   TexCoord.y ) ).g;
    bloom.b = texture( uHdrBuffer2, vec2( TexCoord.x - uChromaticAberration,	TexCoord.y ) ).b;
    
    //vec3 scene = texture( uHdrBuffer3, TexCoord.st ).rgb;
    vec3 scene = vec3( 0.0 );
    scene.r = texture( uHdrBuffer3, vec2( TexCoord.x + uChromaticAberration,	TexCoord.y ) ).r;
    scene.g = texture( uHdrBuffer3, vec2( TexCoord.x + 0.0,                     TexCoord.y ) ).g;
    scene.b = texture( uHdrBuffer3, vec2( TexCoord.x - uChromaticAberration,	TexCoord.y ) ).b;
    
    float aberration = uChromaticAberration;

    //vec3 blommGodraysMix =  mix( trail, bloom, uMixBloomGodrays );
    
    scene += trail;
    
    if ( uIsBloom == true || uIsGodrays == true ) scene += bloom;

    // Test float from GL_RGBA32F_ARB GL_RGBA16F_ARB with clamp
    /*if ( hdrColor.r > 1.1 || hdrColor.g > 1.1 || hdrColor.b > 1.1 )
    {
        gl_FragColor = vec4( vec3(1.0, 0.0, 0.0), 1.0 );
    }
    else gl_FragColor = vec4( hdrColor, 1.0 );*/
    
    // reinhard
    result = scene / ( scene + vec3( 1.0 ) );
    
    //exposure
    if ( uIsBloom == true || uIsGodrays == true ) result = vec3( 1.0 ) - exp( -scene * uExposure );
    else result = scene;

    // also gamma correct while we're at it
    //Default standart value for uGamma = 2.2;
    if ( uIsgamma )
    {
        result = pow( result, vec3( 1.0 / uGamma ) );
    }

    oFragColor0 = vec4( result, 1.0 );
}
