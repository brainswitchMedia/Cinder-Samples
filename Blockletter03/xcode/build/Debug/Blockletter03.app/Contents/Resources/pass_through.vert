#version 330 core

uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
in vec2			ciTexCoord0;

out vec2		TexCoord;

void main()
{	
    TexCoord    = ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
}

