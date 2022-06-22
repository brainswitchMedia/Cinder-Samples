#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform mat4	ciModelMatrix;
uniform mat4    ciProjectionMatrix;

in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;
in vec2		ciTexCoord0;

out VertexData {
	vec4 position;
    vec4 worldPos;
	vec3 normal;
	vec2 texCoord;
    mat4 projectionMatrix;

} vVertexOut;

void main()
{
	vVertexOut.position = ciModelView * ciPosition;
	vVertexOut.normal = ciNormalMatrix * ciNormal;
    vVertexOut.worldPos = ciModelMatrix * ciPosition ;
	vVertexOut.texCoord = ciTexCoord0;
    vVertexOut.projectionMatrix = ciProjectionMatrix;


	gl_Position = ciModelViewProjection * ciPosition;
}
