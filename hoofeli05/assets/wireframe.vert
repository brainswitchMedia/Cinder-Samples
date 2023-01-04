#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4    ciProjectionMatrix;
uniform mat4	ciViewMatrix;
uniform mat4	ciModelMatrix;
uniform mat4	ciModelView;

uniform vec3    uLightPosition;
uniform float   uSize;

in vec4         ciPosition;
in vec3         ciNormal;

out VertexData{
    vec4 vPosition;
    vec4 vLigthPosition;
    mat4 vProjectionMatrix;
    vec4 vVertexWorldSpace;
} VertexOut;


void main()
{
    gl_Position = ciModelViewProjection * ciPosition;
    vec3 position = ciPosition.xyz * uSize;
    VertexOut.vPosition = ciModelView * vec4( position, 1.0 );
    VertexOut.vLigthPosition = ciViewMatrix * vec4( uLightPosition, 1.0 );
    VertexOut.vProjectionMatrix = ciProjectionMatrix;
    VertexOut.vVertexWorldSpace = ciModelMatrix * ciPosition;
}
