#version 330 core

uniform mat4	ciModelViewProjection;
in vec4         ciPosition;



void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
}
