#version 330 core

uniform mat4    ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;
uniform mat4    ciProjectionMatrix;
uniform mat4	ciViewMatrix;
uniform mat4	ciModelMatrix;
uniform int     uVertical;
uniform vec3    uLightPosition;

in vec4			ciPosition;
in vec3         ciNormal;
in vec2			ciTexCoord0;


out VertexData {
    float   V;
    float   stripeType;
    vec3    normal;
    vec4    position;
    vec2    texCoord;
    vec4    vertex;
    mat4    projectionMatrix;
    vec4    vertexWorldSpace;
} vVertexOut;


void main()
{
    if ( uVertical == 0 )
    {
        vVertexOut.V = ciTexCoord0.t;
        vVertexOut.stripeType = 10.0;
    }
    else if ( uVertical == 1 )
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
    vVertexOut.vertex = gl_Position;
    vVertexOut.projectionMatrix = ciProjectionMatrix;
    vVertexOut.vertexWorldSpace = ciModelMatrix * ciPosition;
    gl_Position = ciModelViewProjection * ciPosition;
}
