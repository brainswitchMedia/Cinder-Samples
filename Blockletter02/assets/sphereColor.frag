#version 330 core

uniform sampler2D   uTexStripes;
uniform sampler2D   uTexGradiant;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform vec3        uColor;
uniform float       uTime;
uniform float       uBrightness;


in VertexData {
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
    float translulencyDist = pow( depthFF - dist, 40 );
    
    float stripes = texture( uTexStripes, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t + uTime ) ).r;
    float gradiant = texture( uTexGradiant, vec2( vVertexIn.texCoord.st ) ).r;

    vec3 finalColor = clamp( translulencyDist, 0.0, 1.0 ) * pow( vVertexIn.texCoord.t, 0.1) * uColor;// * gradiant;
    finalColor = finalColor * stripes;
    
    //oFragColor0.rgb = vec3( stripes * uColor * gradiant * uBrightness );
    oFragColor0.rgb = finalColor * gradiant * uBrightness;
    oFragColor0.a = 1.0;
    oFragColor1.rgb = uColor*0.5;
    oFragColor1.a = 1.0;
    oFragColor2.rgb = finalColor;
    oFragColor2.a = 1.0;

}
