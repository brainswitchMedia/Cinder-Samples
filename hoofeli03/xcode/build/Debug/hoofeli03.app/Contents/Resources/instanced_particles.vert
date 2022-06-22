#version 330 core

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat4	ciModelMatrix;
uniform mat4	ciViewMatrix;
uniform mat4    ciProjectionMatrix;

//in vec3			aInstancePosition;
in vec3         aInstanceParticlesData;
in mat4         aInstanceMatrix;

in vec4         ciPosition;
in vec3         ciNormal;
in vec2         ciTexCoord0;

out vec3		Normal;
out vec2        TexCoord;
out mat4        vProjectionMatrix;
out vec4        vVertexWorldSpace;
out float       vSize;


void main()
{    
    // Use transpose
    Normal              = mat3( transpose(inverse( aInstanceMatrix ) )) * ciNormal;
    TexCoord            = ciTexCoord0;
    vProjectionMatrix   = ciProjectionMatrix;
    vVertexWorldSpace   = aInstanceMatrix * ciPosition;
    vSize               = aInstanceParticlesData.x;

    gl_Position         =  ciProjectionMatrix * ciViewMatrix * aInstanceMatrix * ciPosition;
    //gl_Position       =  ciProjectionMatrix * ciViewMatrix * ciModelMatrix * ciPosition;
    //gl_Position       = ciModelViewProjection * ( ciPosition + vec4( aInstancePosition, 0 ) );

}
