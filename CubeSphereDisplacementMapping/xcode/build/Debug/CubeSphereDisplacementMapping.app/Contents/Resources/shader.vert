/* 
 Tessellation Shader from Philip Rideout
 
 "Triangle Tessellation with OpenGL 4.0"
 http://prideout.net/blog/?p=48 */

#version 400

uniform mat3        ciNormalMatrix;
uniform mat4        ciViewMatrix;

uniform samplerCube uHeightMap;

in vec4             ciPosition;
in vec3             ciNormal;
in vec3             ciTangent;
in vec2             ciTexCoord0;

out vec3            vPosition;
out vec3            vNormal;
out vec3            vTangent;
out vec2            vTexCoord;


void main()
{
    vPosition = ciPosition.xyz;
    vNormal = ciNormal.xyz;
    vTangent = ciTangent.xyz;
    vTexCoord = ciTexCoord0;
}
