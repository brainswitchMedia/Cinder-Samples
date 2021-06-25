#version 330

layout (location = 0) in vec3 vPosition;
layout (location = 1) in float fLifeTime;
layout (location = 2) in vec3 vVelocity;
layout (location = 3) in float fSize;
layout (location = 4) in float fAge;
layout (location = 5) in int iType;

out vec3 vPositionPass;
out float fLifeTimePass;
out vec3 vVelocityPass;
out float fSizePass;
out float fAgePass;
out int iTypePass;

void main()
{
    vPositionPass = vPosition;
    vVelocityPass = vVelocity;
    fLifeTimePass = fLifeTime;
    fSizePass = fSize;
    fAgePass = fAge;
    iTypePass = iType;
}
