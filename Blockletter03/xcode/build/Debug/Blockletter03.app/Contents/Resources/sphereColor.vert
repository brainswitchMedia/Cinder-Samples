#version 330 core

uniform mat4    ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform mat4    ciProjectionMatrix;
uniform mat4	ciViewMatrix;
uniform mat4	ciModelMatrix;
uniform vec3    uLightPosition;

uniform float   uSize;

in vec4			ciPosition;
in vec3         ciNormal;
in vec2			ciTexCoord0;


out VertexData {
    vec3    normal;
    vec4    position;
    vec2    texCoord;
    vec4    vertex;
    mat4    projectionMatrix;
    vec4    vertexWorldSpace;
} vVertexOut;


void main()
{
    vec4 position = ciPosition;
    position.xyz = position.xyz * uSize;
    vVertexOut.position = position;
    vVertexOut.normal = ciNormalMatrix * ciNormal;
    vVertexOut.texCoord = ciTexCoord0;
    vVertexOut.vertex = position;
    vVertexOut.projectionMatrix = ciProjectionMatrix;
    vVertexOut.vertexWorldSpace = ciModelMatrix * position;
    gl_Position = ciModelViewProjection * position;
}
