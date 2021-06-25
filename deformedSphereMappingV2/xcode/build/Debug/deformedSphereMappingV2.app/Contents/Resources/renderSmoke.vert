#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in float fLifeTime;
layout (location = 3) in float fSize;
layout (location = 4) in float fAge;
layout (location = 5) in int iType;


uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;


// To Fragment Shader
out VS_OUT {
    float   Transp;
    float   Radius;
    int     Particletype;
} vs_out;

#define PI 3.1415926



void main() {
    
    float distfromcenter = length( vPosition );
    
    gl_Position = ciModelView * vec4(vPosition, 1.0);

    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.02-0.7 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // (fLifeTime - 0) -> (0 - 1)
    
    float sizecoef = ( fAge-fLifeTime ) / fAge ;
    vs_out.Transp = clamp ( sizecoef, 0.0, 0.9 );
    vs_out.Radius = ( 0.005 + fSize * 0.5 * sin( PI*sizecoef ) ) * clamp( distfromcenter/3.0, 0.0, 1.0 );

    vs_out.Particletype = iType;
    
	
}
