#version 330 core

uniform sampler2D   uTexGradiant;
uniform sampler2D   uTexStripes;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform float       uTime;
uniform float       uBrightness;
uniform int         uTexture;

in VertexData {
    float   V;
    float   stripeType;
    vec3    normal;
    vec4    position;
    vec2    texCoord;
    vec4    vertex;
    mat4    projectionMatrix;
    vec4    vertexWorldSpace;
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
    float translulencyDist = pow( depthFF - dist, 20 );
    
    
    float gradiant = texture( uTexGradiant, vVertexIn.texCoord.st ).r;
    //float stripes = texture( uTexStripes, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t - uTime ) ).r;
    float stripes = texture( uTexStripes, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t ) ).r;

    
    vec3 colorBlue = vec3( 0.3, 0.315, 0.315 );
    vec3 colorYellow = vec3( 1.00, 0.45, 0.0 );
    vec3 orangeColor = vec3( 1.00, 0.2, 0.02 );
    vec3 mixColor= mix( orangeColor, colorBlue, pow(vVertexIn.texCoord.t, 2.0) );
   
    vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * uBrightness * pow( vVertexIn.texCoord.t, 0.1) * orangeColor;// * gradiant;
    //if ( uTexture == 1 ) finalColor = finalColor * stripes;
    finalColor = finalColor * stripes;
    
    //vec3 finalColor = translulencyDist * vVertexIn.texCoord.t * stripes * colorYellow;

    oFragColor0.rgb = finalColor;
    oFragColor0.a = 1.0;
    oFragColor1.rgb = finalColor;
    oFragColor1.a = 1.0;
    oFragColor2.rgb = finalColor;
    oFragColor2.a = 1.0;
    //oColor = vec4( vVertexIn.texCoord.t, vVertexIn.texCoord.t, vVertexIn.texCoord.t, 1.0 );

}
