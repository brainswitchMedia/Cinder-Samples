#version 330 core

layout( points ) in;
layout( triangle_strip, max_vertices = 4 ) out;

uniform mat4 ciModelViewProjection;
uniform mat4 ciProjectionMatrix;

uniform float Time;


// From Vertex Shader
in VS_OUT {
    float   Transp;
    float   Radius;
    int     Particletype;
} gs_in[];


// To Fragment Shader
out float       fTransp; // Output to fragment shader
out vec2        texCoord;
out vec4        vertexPosition;
flat out int    particleType;

// Geom Shader
float           uRadius = gs_in[0].Radius;

const float     PI = 3.1415926;
const float     twoPI = 6.283185306;


void main()
{
    particleType    = gs_in[0].Particletype; // Point has only one vertex
    
    if(particleType != 0)
    {
        fTransp = gs_in[0].Transp; // Point has only one vertex
    
        float cosinusRadius = 1.0;
        float sinusRadius   = 0.0;
    
        vec4 vert0 = vec4( -uRadius, uRadius, 0.0, 0.0);
        vec4 vert1 = vec4( uRadius, uRadius, 0.0, 0.0);
        vec4 vert2 = vec4( uRadius, -uRadius, 0.0, 0.0);
        vec4 vert3 = vec4( -uRadius, -uRadius, 0.0, 0.0);
    
        // v2(-r,r):uv0  ... v3(r,r):uv1
        //      .                 .
        //      .        .        .
        //      .                 .
        // v0(-r,-r):uv3 ... v1(r,-r):uv2
    
        // Define UV coordinates
        vec2 uv0 = vec2(0.0, 0.0); // --
        vec2 uv1 = vec2(1.0, 0.0); // +-
        vec2 uv2 = vec2(1.0, 1.0); // ++
        vec2 uv3 = vec2(0.0, 1.0); // -+
    
        // The first triangle
        gl_Position = ciProjectionMatrix * (gl_in[0].gl_Position + vert3);
        vertexPosition = gl_Position;
        texCoord = uv0;
        EmitVertex();
        gl_Position =  ciProjectionMatrix * (gl_in[0].gl_Position + vert2);
        vertexPosition = gl_Position;
        texCoord = uv1;
        EmitVertex();
        gl_Position =  ciProjectionMatrix * (gl_in[0].gl_Position + vert0);
        vertexPosition = gl_Position;
        texCoord = uv3;
        EmitVertex();
        // The second triangle
        gl_Position =  ciProjectionMatrix * (gl_in[0].gl_Position + vert1);
        vertexPosition = gl_Position;
        texCoord = uv2;
        EmitVertex();
        // Close the primitive
        EndPrimitive();
    }
}
