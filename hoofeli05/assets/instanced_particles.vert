#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat4	ciModelMatrix;
uniform mat4	ciViewMatrix;
uniform mat4    ciProjectionMatrix;
uniform mat3	ciNormalMatrix;

in vec3			aInstancePosition;
in vec3         aInstanceLightPosition;
in mat4         aInstanceMatrix;
in int          aType;

in vec4         ciPosition;
in vec3         ciNormal;
in vec2         ciTexCoord0;


out VertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoord;
    vec4 light;
    mat4 projectionMatrix;
    vec4 vertexWorldSpace;
    vec4 vertex;
    flat int  type;

} vVertexOut;


void main()
{
    vVertexOut.position = ciViewMatrix * aInstanceMatrix * ciPosition;
    vVertexOut.light = ciViewMatrix * vec4( aInstanceLightPosition, 1.0 );
    vVertexOut.normal = ciNormalMatrix * ciNormal;
    vVertexOut.texCoord = ciTexCoord0;
    vVertexOut.projectionMatrix = ciProjectionMatrix;
    vVertexOut.vertexWorldSpace = aInstanceMatrix * ciPosition;
    gl_Position =  ciProjectionMatrix * ciViewMatrix * aInstanceMatrix * ciPosition;
    vVertexOut.vertex = gl_Position;
    vVertexOut.type = aType;
}
