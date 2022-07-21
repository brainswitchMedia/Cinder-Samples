#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat4	ciModelMatrix;
uniform mat4	ciViewMatrix;
uniform mat3	ciNormalMatrix;
uniform mat4    ciProjectionMatrix;

uniform float	uRadius;

in vec4         ciPosition;
in vec3         ciNormal;
in vec2         ciTexCoord0;

out vec3		Normal;
out vec2        TexCoord;
out vec4        vModelPosition;
out mat4        vProjectionMatrix;
out vec4        vVertexWorldSpace;

void main()
{
    vec4 vertex = ciPosition;
    vertex.rgb *= uRadius;
    
    //Normal              = ciNormal;
    // Work with transpose
    Normal              = mat3(transpose(inverse(ciModelMatrix))) * ciNormal;
    //Normal              = mat3(ciModelMatrix) * ciNormal;

    TexCoord            = ciTexCoord0;
    vModelPosition      = ciModelMatrix * ciPosition;
    vProjectionMatrix   = ciProjectionMatrix;
    
    // calculate view space position (required for lighting)
    vVertexWorldSpace   = ciModelMatrix * ciPosition;
    
    
    
    vVertexOut.position = ciModelView * ciPosition;
    vVertexOut.ligth = ciViewMatrix * vec4(uLightPosition, 1.0);
    vVertexOut.normal = ciNormalMatrix * ciNormal;
    vVertexOut.color = ciColor;
    vVertexOut.texCoord = ciTexCoord0;
    
    gl_Position         = ciModelViewProjection * vertex;

    
}


