#version 330 core

uniform mat4    ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform int     vertical;

in vec4			ciPosition;
in vec3         ciNormal;
in vec2			ciTexCoord0;


out VertexData {
    float   V;
    float   stripeType;
    vec3    normal;
    vec4    position;
    vec2    texCoord;
    
} vVertexOut;


void main()
{
    if ( vertical == 0 )
    {
        vVertexOut.V = ciTexCoord0.t;
        vVertexOut.stripeType = 10.0;
    }
	else if ( vertical == 1 )
    {
        vVertexOut.V = ciTexCoord0.s;
        vVertexOut.stripeType = 0.0;
    }
	else 
    {
        vVertexOut.V = ciTexCoord0.t + ciTexCoord0.s;
        vVertexOut.stripeType = 0.0;
    }  
    
    vVertexOut.position = ciPosition;
    vVertexOut.normal = ciNormalMatrix * ciNormal;
    vVertexOut.texCoord = ciTexCoord0;
    gl_Position = ciModelViewProjection * ciPosition;
}
