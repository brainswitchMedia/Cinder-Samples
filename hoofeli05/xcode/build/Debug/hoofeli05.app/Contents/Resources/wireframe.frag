#version 330 core

uniform float       uBrightness;
uniform float       uPower1;
uniform float       uPower2;
uniform int         uLineStyle;
uniform vec3        uColor;
uniform vec4        uLightModelView;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;

in VertexData{
    vec3 mBaricentric;
    vec2 mTexCoord;
    vec3 mPosition;
    vec3 mNormal;
    vec3 mLigthPosition;
    mat4 mProjectionMatrix;
    vec4 mVertexWorldSpace;
    vec4 mVertex;
} VertexIn;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


float edgeFactor()
{
    vec3 a3 = vec3(0.0);
    
    // normal line
    if ( uLineStyle == 1 )
    {
        vec3 d = fwidth( VertexIn.mBaricentric );
        a3 = smoothstep( vec3(0.0), d * 1.0, VertexIn.mBaricentric );
    }
    
    // Stippled line
    // one way to calculate interpolation factor
    if ( uLineStyle == 2 ||  uLineStyle == 3 )
    {
        float f = VertexIn.mBaricentric.x;
        if( VertexIn.mBaricentric.x < min( VertexIn.mBaricentric.y, VertexIn.mBaricentric.z ) )
        {
            f = VertexIn.mBaricentric.y;
        }
        
        const float PI = 3.14159265;
        float stipple = 0.0;
        
        if ( uLineStyle == 2 ) { stipple = pow( clamp( 20 * sin( 10*f ), 0, 1 ), 1 ); }   // croix
        if ( uLineStyle == 3 ) { stipple = pow( clamp( 1, 0, 1 ), 1 ); }                  // ligne 1
        if ( uLineStyle == 3 ) { stipple = pow( clamp( sin( 20*f*PI ), 0, 1 ), 1 ); }     // ligne 2
        
        float thickness = 1.0 * stipple;
        
        vec3 d = fwidth( VertexIn.mBaricentric );
        a3 = smoothstep( vec3( 0.0 ), d * thickness, VertexIn.mBaricentric );
    }
    
    return min( min( a3.x, a3.y), a3.z );
}


void main()
{
    float DepthLigth = -( uWorldtoLightMatrix * VertexIn.mVertexWorldSpace ).z;
    float A = VertexIn.mProjectionMatrix[2].z;
    float B = VertexIn.mProjectionMatrix[3].z;
    float zNear = - B / ( 1.0 - A );
    float zFar = B / ( 1.0 + A );
    float depthFF = 0.5 * ( -A * DepthLigth + B ) / DepthLigth + 0.5;
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // 0 - 200  ->  0 - 1
    float distToLigth = length( uWorldtoLightMatrix * VertexIn.mVertexWorldSpace );
    float dist = (( distToLigth - 0.0 ) / ( uDistanceConverstion - 0.0 )) * ( 1.0 - 0.0 ) + 0.0;
    float translulencyDist = pow( depthFF - dist, 20.0 );

    float distToCenterLight = length( uLightModelView.xyz - VertexIn.mPosition );
    //vec3 direction =  normalize( uLightModelView.xyz - VertexIn.mPosition );
    //float dotProduct = max( dot( VertexIn.mNormal, -direction ), 0.0 );

    float fEdgeIntensity = 1.0 - edgeFactor();
    vec3 vFaceColor = vec3( 0.0, 0.0, 0.0 );
    vec3 vEdgeColor = uColor;
    
    vec3 colorMix = mix( vFaceColor, vEdgeColor, fEdgeIntensity );

    // Compute viewport coords for dithering
    vec3 ndc = VertexIn.mVertex.xyz / VertexIn.mVertex.w;
    vec2 viewportCoord = ndc.xy * 0.5 + 0.5;
    
    // 0.7 for large resolution
    //vec3 finalColor = vec3( colorMixDithering ) * translulencyDist;
    
    // For only spheres, phase 1
    vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( colorMix * 0.7 * ( 0.5 - pow( VertexIn.mTexCoord.s, 1.5 ) )
                    + vec3( uBrightness, uBrightness, uBrightness ) * pow( 1.0 / distance( VertexIn.mLigthPosition, VertexIn.mPosition ), uPower2 )
                    + vec3( 1.0, 1.0, 1.0 ) * clamp( ( distToCenterLight ), 0.0, 1.0 ) * uPower1 );
    
    
    // For cones & spheres phase 2
    /*vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( colorMix * 0.7 * ( pow( VertexIn.mTexCoord.s, 1.5 ) )
                      + vec3( uBrightness, uBrightness, uBrightness ) * pow( distance( VertexIn.mLigthPosition, VertexIn.mPosition ), uPower2 )
                     + vec3( 1.0, 1.0, 1.0 ) * clamp( ( distToCenterLight ), 0.0, 1.0 ) * uPower1 );*/
    

    
    oFragColor0.rgb = vec3( finalColor.r, finalColor.r, finalColor.r ) ;
    oFragColor1.rgb = vec3( 0.0, 0.0, 0.0 );
    oFragColor2.rgb = vec3( finalColor.r );
    
    oFragColor0.a = 1.0;
    oFragColor1.a = 1.0;
    oFragColor2.a = 1.0;
}
