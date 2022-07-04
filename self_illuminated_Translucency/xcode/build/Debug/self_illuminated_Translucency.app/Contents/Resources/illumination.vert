#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform float   uSize;

in vec4		ciPosition;
in vec3		ciNormal;


out VertexData {
	vec4 position;
	vec3 normal;

} vVertexOut;

void main()
{
    vec3 position = ciPosition.xyz * uSize;
	vVertexOut.position = ciModelView * vec4( position, 1.0 );
	vVertexOut.normal = ciNormalMatrix * ciNormal;
	gl_Position = ciModelViewProjection * ciPosition;
}
