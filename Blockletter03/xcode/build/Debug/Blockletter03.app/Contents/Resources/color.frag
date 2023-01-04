#version 330 core

uniform float       uBrightness;
uniform float       uPower1;
uniform int         uLineStyle;
uniform vec3        uColor;
uniform vec4        uLightModelView;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform bool        uFront;
uniform float       uCircleSize;


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


// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    /*vec3 tangentNormal = texture( normalMap, TexCoords ).xyz * 2.0 - 1.0;
     
     vec3 Q1  = dFdx( WorldPos );
     vec3 Q2  = dFdy( WorldPos );
     vec2 st1 = dFdx( TexCoords );
     vec2 st2 = dFdy( TexCoords );
     
     vec3 N   = normalize( Normal );
     vec3 T  = normalize( Q1*st2.t - Q2*st1.t );
     vec3 B  = -normalize( cross( N, T ));
     mat3 TBN = mat3( T, B, N );
     
     return normalize( TBN * tangentNormal );*/
    return vec3( 0.0 );
}


float edgeFactor()
{
    vec3 a3 = vec3(0.0);
    
    // normal line
    if ( uLineStyle == 1 )
    {
        vec3 d = fwidth( VertexIn.mBaricentric );
        a3 = smoothstep( vec3(0.0), d * 3.0, VertexIn.mBaricentric );
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
    float translulencyDist = pow( depthFF - dist, 30.0 );
    
    float distToCenterLight = length( uLightModelView.xyz - VertexIn.mPosition );
    
    float fEdgeIntensity = 1.0 - edgeFactor();
    vec3 vFaceColor = vec3( 0.0, 0.0, 0.0 );
    vec3 vEdgeColor = uColor;
    vec3 colorMix = mix( vFaceColor, vEdgeColor, fEdgeIntensity );
    
    //vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( 2.0 * colorMix + uColor * pow( ( distToCenterLight ), uPower1 ));
    //vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( 3.0 * colorMix + uColor * pow( ( 1.0 - distToCenterLight ), uPower1 ));
    vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( 2.5 * colorMix + uColor * pow( ( 1.0 - sin( uCircleSize * distToCenterLight ) ), uPower1 ));

    
    // trails
    //vec3 finalColor =  clamp( translulencyDist, 0.0, 1.0 ) * colorMix;
    vec3 finalColorTrails = vec3( 0.0, 0.0, 0.0 ) ;
    if ( uFront == true ) finalColorTrails = colorMix * 0.5 ;
    //if ( uFront == true ) finalColorTrails = vec3( 1.00, 0.2, 0.02  );
    
    
    oFragColor0.rgb = clamp(finalColor, -5.0, 5.0);
    //oFragColor1.rgb = 3.0 * finalColorTrails;
    oFragColor1.rgb = vec3( 0.0, 0.0, 0.0 );
    
    oFragColor2.rgb = vec3( 0.0, 0.0, 0.0 );
    
    oFragColor0.a = 1.0;
    oFragColor1.a = 1.0;
    oFragColor2.a = 0.0;
}
