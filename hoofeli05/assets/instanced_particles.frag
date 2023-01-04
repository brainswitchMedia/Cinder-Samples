#version 330 core

uniform sampler2D	uTex0;

uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform float       uPower;
uniform float       uBrightness;
uniform int         uMode;
uniform float       uTime;
uniform float       uPowerTranslulencyDist;

in VertexData	{
    vec4 position;
    vec3 normal;
    vec2 texCoord;
    vec4 light;
    mat4 projectionMatrix;
    vec4 vertexWorldSpace;
    vec4 vertex;
    flat int  type;
} vVertexIn;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


void main()
{
    float DepthLigth = -( uWorldtoLightMatrix * vVertexIn.vertexWorldSpace ).z;
    float A = vVertexIn.projectionMatrix[2].z;
    float B = vVertexIn.projectionMatrix[3].z;
    float zNear = - B / ( 1.0 - A );
    float zFar = B / ( 1.0 + A );
    float depthFF = 0.5 * ( -A * DepthLigth + B ) / DepthLigth + 0.5;
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // 0 - 200  ->  0 - 1
    float distToLigth = length( uWorldtoLightMatrix * vVertexIn.vertexWorldSpace );
    float dist = (( distToLigth - 0.0 ) / ( uDistanceConverstion - 0.0 )) * ( 1.0 - 0.0 ) + 0.0;
    float translulencyDist = pow( depthFF - dist, uPowerTranslulencyDist );
    
    // diffuse coefficient
    vec3 diffuse = vec3( 0.0 );
    
    if( uMode == 0 ) {
        float color = uBrightness;
        diffuse = color * vec3( 1.00, 0.15, 0.02 ) * clamp ( pow( distance( vVertexIn.light.xyz, vVertexIn.position.xyz ), uPower ) * translulencyDist, 0.0, 1.0);;
    }
    else if ( uMode == 1 )
    {
       // vec3( 1.00, 0.45, 0.0 ) // yellow
        vec3 orangeColor = vec3( 1.00, 0.15, 0.02 );
        vec3 greyColor = vec3( 0.9, 0.95, 0.95 );

        vec3 colorType = greyColor;
        if ( vVertexIn.type == 1 ) colorType = orangeColor * 0.7;
        
        float color = uBrightness * texture( uTex0, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t + uTime ) ).r;
        
        diffuse = color * colorType * clamp ( translulencyDist * pow( distance( vVertexIn.light.xyz, vVertexIn.position.xyz ), uPower ), 0.0, 1.0) ;
    }
    
    // final color
    oFragColor0 = vec4( diffuse , 1.0 );
    oFragColor1 = vec4( 0.0, 0.0, 0.0 , 1.0 );
    oFragColor2 = vec4( diffuse , 1.0 );
}
