#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4    ciProjectionMatrix;
uniform mat4	ciViewMatrix;
uniform mat4	ciModelMatrix;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;

uniform vec3    uLightPosition;
uniform float   uSize;

in vec4         ciPosition;
in vec3         ciNormal;

out VertexData{
    vec4 vPosition;
    vec3 vLigthPosition;
    mat4 vProjectionMatrix;
    vec4 vVertexWorldSpace;
} VertexOut;


void main()
{
    vec4 position = ciPosition;
    position.xyz = position.xyz * uSize;
    gl_Position = ciModelViewProjection * position;
    VertexOut.vPosition = ciModelView * position; // view space
    VertexOut.vLigthPosition = normalize( ciNormalMatrix * ciNormal ); // view space ligth Position stored in the ciNormal buffer
    VertexOut.vProjectionMatrix = ciProjectionMatrix;
    VertexOut.vVertexWorldSpace = ciModelMatrix * position;
}
