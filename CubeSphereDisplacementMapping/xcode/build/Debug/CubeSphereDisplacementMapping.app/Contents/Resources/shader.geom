/* 
 Tessellation Shader from Philip Rideout
 
 "Triangle Tessellation with OpenGL 4.0"
 http://prideout.net/blog/?p=48 */

#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4    ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3    ciNormalMatrix;

in vec3         tePosition[3];
in vec3         tePatchDistance[3];
in vec2         teTexCoord[3];
in vec3         teNormal[3];
in vec3         teTangent[3];
in vec3         teTexCoord3D[3];

out vec3        gFacetNormal; 	// calculate view space normal (required for lighting & normal mapping)
out vec3        gFaceTangent;
out vec3        gFaceBitangent;
out vec2        gTexCoord;
out vec4        gVertexViewSpace; // calculate view space position (required for lighting)
out vec3        gTexCoord3D;
out vec3        gVertexModelSpace;


void main()
{
    gl_Position     = ciModelViewProjection * gl_in[0].gl_Position;
    gVertexViewSpace =  ciModelView * gl_in[0].gl_Position;
    gVertexModelSpace = gl_in[0].gl_Position.xyz;
    gFacetNormal = normalize( ciNormalMatrix * teNormal[0] );
    gFaceTangent = normalize( ciNormalMatrix * teTangent[0] );
    gFaceBitangent = normalize( cross( gFaceTangent, gFacetNormal ) );
    gTexCoord3D = teTexCoord3D[0];
    gTexCoord = teTexCoord[0];

    EmitVertex();
    
    gl_Position     = ciModelViewProjection * gl_in[1].gl_Position;
    gVertexViewSpace = ciModelView*gl_in[1].gl_Position;
    gVertexModelSpace = gl_in[1].gl_Position.xyz;
    gFacetNormal = normalize( ciNormalMatrix*teNormal[1] );
    gFaceTangent = normalize( ciNormalMatrix*teTangent[1] );
    gFaceBitangent = normalize( cross( gFaceTangent, gFacetNormal ) );
    gTexCoord3D = teTexCoord3D[1];
    gTexCoord = teTexCoord[1];

    EmitVertex();
    
    gl_Position     = ciModelViewProjection *  gl_in[2].gl_Position;
    gVertexViewSpace = ciModelView*gl_in[2].gl_Position;
    gVertexModelSpace = gl_in[2].gl_Position.xyz;
    gFacetNormal = normalize( ciNormalMatrix*teNormal[2] );
    gFaceTangent = normalize( ciNormalMatrix*teTangent[2] );
    gFaceBitangent = normalize( cross( gFaceTangent, gFacetNormal ) );
    gTexCoord3D = teTexCoord3D[2];
    gTexCoord = teTexCoord[2];

    EmitVertex();
    
    EndPrimitive();
}
