#version 400

uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
in vec2			ciTexCoord0;
out vec2		TexCoord0;

void main( void )
{
    
    mat4 m		= ciModelViewProjection;
    vec4 p		= ciPosition;
    
    // pass texture coordinates
    TexCoord0 = ciTexCoord0;
    
    gl_Position = m * p;
    
}
