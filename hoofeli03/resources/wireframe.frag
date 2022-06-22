#version 330 core

uniform float uBrightness;
uniform int uLineStyle;
uniform vec3 uColor;

in vec3 baricentric;

layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;

float edgeFactor()
{
    vec3 a3 = vec3(0.0);
    
    // Normal line
    if ( uLineStyle == 1 )
    {
        vec3 d = fwidth( baricentric );
        a3 = smoothstep( vec3(0.0), d * 1.0, baricentric );
    }
    
    // Stippled line
    // One way to calculate interpolation factor
    if ( uLineStyle == 2 ||  uLineStyle == 3 )
    {
        float f = baricentric.x;
        if( baricentric.x < min(baricentric.y, baricentric.z) )
        {
            f = baricentric.y;
        }
        
        const float PI = 3.14159265;
        float stipple = 0.0;
        
        if ( uLineStyle == 2 ) { stipple = pow( clamp( 20 * sin( 10*f ), 0, 1 ), 1 ); }   // croix
        if ( uLineStyle == 3 ) { stipple = pow( clamp( 1, 0, 1 ), 1 ); }                  // ligne 1
        if ( uLineStyle == 3 ) { stipple = pow( clamp( sin( 20*f*PI ), 0, 1 ), 1 ); }     // ligne 2
        
        float thickness = 1.0 * stipple;
        
        vec3 d = fwidth( baricentric );
        a3 = smoothstep( vec3( 0.0 ), d * thickness, baricentric );
    }
    
    return min( min( a3.x, a3.y), a3.z );
}


void main()
{
    float fEdgeIntensity = 1.0 - edgeFactor();
    vec3 vFaceColor = vec3( 0.0, 0.0, 0.0 );
    vec3 vEdgeColor = uColor;
    
    vec3 colorMix = mix( vFaceColor, vEdgeColor, fEdgeIntensity );

	oFragColor0.rgb = colorMix * uBrightness;
	oFragColor0.a = 1.0;
    
    oFragColor1.rgb = vec3( 0.0, 0.0, 0.0 );
    oFragColor1.a = 1.0;
}
