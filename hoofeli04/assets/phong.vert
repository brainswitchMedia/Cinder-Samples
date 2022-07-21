#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform mat4	ciViewMatrix;

uniform vec3    uLightPosition;

in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;
in vec2		ciTexCoord0;

out VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
    vec4 ligth;
} vVertexOut;

void main()
{
	vVertexOut.position = ciModelView * ciPosition;
    vVertexOut.ligth = ciViewMatrix * vec4(uLightPosition, 1.0);
	vVertexOut.normal = ciNormalMatrix * ciNormal;
	vVertexOut.color = ciColor;
	vVertexOut.texCoord = ciTexCoord0;

	gl_Position = ciModelViewProjection * ciPosition;
}
