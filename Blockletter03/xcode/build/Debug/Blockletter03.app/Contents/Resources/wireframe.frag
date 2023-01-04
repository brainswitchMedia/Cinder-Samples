#version 330 core

uniform sampler2D	uTexNormal;
uniform sampler2D	uTexRoughness;
uniform float       uBrightness;
uniform float       uPower1;
uniform int         uLineStyle;
uniform vec3        uColor;
uniform vec4        uLightModelView;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform bool        uFront;
uniform vec3        uLightPosition;
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
    vec3 tangentNormal = texture( uTexNormal, VertexIn.mTexCoord ).xyz * 2.0 - 1.0;
    
    vec3 Q1  = dFdx( VertexIn.mPosition.xyz );
    vec3 Q2  = dFdy( VertexIn.mPosition.xyz );
    vec2 st1 = dFdx( VertexIn.mTexCoord );
    vec2 st2 = dFdy( VertexIn.mTexCoord );
    
    vec3 N   = normalize( VertexIn.mNormal );
    vec3 T  = normalize( Q1*st2.t - Q2*st1.t );
    vec3 B  = -normalize( cross( N, T ));
    mat3 TBN = mat3( T, B, N );
    
    return normalize( TBN * tangentNormal );
}

float edgeFactor()
{
    vec3 a3 = vec3(0.0);
    
    // normal line
    if ( uLineStyle == 1 )
    {
        vec3 d = fwidth( VertexIn.mBaricentric );
        a3 = smoothstep( vec3(0.0), d * 4.0, VertexIn.mBaricentric );
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
    float translulencyDist = pow( depthFF - dist, 10.0 );

    float distToCenterLight = length( uLightModelView.xyz - VertexIn.mPosition );

    float fEdgeIntensity = 1.0 - edgeFactor();
    vec3 vFaceColor = vec3(0);
    vec3 vEdgeColor = uColor;
    vec3 colorMix = mix( vFaceColor, vEdgeColor, fEdgeIntensity );

    float roughness = texture( uTexRoughness, VertexIn.mTexCoord ).x;
    
    vec3 normal = getNormalFromMap();
    
    // Option 1 : One unique ligth in view space
    //vec3 ligthPosition = uLightPosition;
    
    // Option 2 : Multiple ligths in view space
    vec3 ligthPosition = VertexIn.mLigthPosition;
    
    vec3 vToCamera = normalize( -VertexIn.mPosition.xyz );
    vec3 light = normalize( ligthPosition - VertexIn.mPosition.xyz );
    
    vec3 reflect = normalize( -reflect( light, normal ) );
    
    // calculate diffuse term
    float fDiffuse = max( dot( normal, light ), 0.0 );
    fDiffuse = clamp( fDiffuse, 0.1, 1.0 );
    
    // calculate specular term
    float fSpecularPower = 20.0;
    float fSpecular = pow( max( dot( reflect, vToCamera ), 0.0 ), fSpecularPower );
    fSpecular = clamp( fSpecular, 0.0, 1.0 );
    
    // specular coefficient
    float specular = 0.0;
    
    float alfa = 1.0;
    
    // trails
    //vec3 finalColor =  clamp( translulencyDist, 0.0, 1.0 ) * colorMix;
    vec3 finalColorTrails = vec3( 0.0, 0.0, 0.0 );
    if ( uFront == true )
    {
        //finalColorTrails = colorMix * 2.0 + 0.8 * uColor;
        finalColorTrails = 1.8 * uColor;
        specular = fSpecular * roughness;
         alfa = 0.3;
    }
    //if ( uFront == true ) finalColorTrails = vec3( 1.00, 0.2, 0.02  );

    //float test = clamp( pow( ( 1.0 - sin( uCircleSize * distToCenterLight ) ), uPower1 ), 0.0, 1.0 ) ;
    vec3 colorTest = vec3( 0.0, 0.0, 0.0 );
    
    float test = 1.0;
    //if ( uCircleSize * distToCenterLight < 1.5708 ) colorTest = vec3( 1.0, 1.0, 1.0 );
    if ( uCircleSize * distToCenterLight < 1.548 + uCircleSize * 0.01 ) test = 0.0;
 
    
    //vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * (colorMix + uColor * pow( ( distToCenterLight ), uPower1 ));
    //vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( 10.0 *  colorMix + uColor * pow( ( 1 - sin( uCircleSize * distToCenterLight ) ), uPower1 )  ) + uColor * vec3( 12.0 ) * specular;
    vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * ( 8.0 * colorMix + uColor * pow( ( 1.0 - sin( uCircleSize * distToCenterLight ) ), uPower1 )  ) + 12.0 * uColor  * specular;


    if ( uFront == true )
    {
        oFragColor0.rgb = clamp( finalColor, -8.0, 8.0 ) * test;
    }
    else
    {
        oFragColor0.rgb = clamp( finalColor, -5.0, 5.0 ) * test;
    }
     
    //oFragColor0.rgb = finalColor;

    oFragColor1.rgb = finalColorTrails * test;
    //oFragColor1.rgb = vec3( 0.0, 0.0, 0.0 );


    oFragColor2.rgb = vec3( 0.0, 0.0, 0.0 );
    
    oFragColor0.a = alfa * test;
    oFragColor1.a = 1.0 * test;
    oFragColor2.a = 0.0;
}
