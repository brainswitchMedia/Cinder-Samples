#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VertexData{
    vec4 vPosition;
    vec4 vLigthPosition;
    mat4 vProjectionMatrix;
    vec4 vVertexWorldSpace;
} VertexIn[3];


out VertexData{
    vec3 mBaricentric;
    vec2 mTexCoord;
    vec3 mPosition;
    vec3 mNormal;
    vec3 mLigthPosition;
    mat4 mProjectionMatrix;
    vec4 mVertexWorldSpace;
    vec4 mVertex;
} VertexOut;


void main()
{
    
    vec3 e0 = VertexIn[0].vPosition.xyz - VertexIn[1].vPosition.xyz;
    vec3 e1 = VertexIn[0].vPosition.xyz - VertexIn[2].vPosition.xyz;

    vec3 normal = normalize( cross( e0, e1 ));
    
    VertexOut.mBaricentric = vec3(1, 0, 0);
    VertexOut.mTexCoord = vec2(0.0, 0.0);
    VertexOut.mPosition = VertexIn[0].vPosition.xyz;
    VertexOut.mNormal = normal;
    VertexOut.mLigthPosition = VertexIn[0].vLigthPosition.xyz;
    VertexOut.mProjectionMatrix = VertexIn[0].vProjectionMatrix;
    VertexOut.mVertexWorldSpace = VertexIn[0].vVertexWorldSpace;
    VertexOut.mVertex = gl_in[0].gl_Position;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

    VertexOut.mBaricentric = vec3(0, 1, 0);
    VertexOut.mTexCoord = vec2(0.5, 0.5);
    VertexOut.mPosition = VertexIn[1].vPosition.xyz;
    VertexOut.mNormal = normal;
    VertexOut.mLigthPosition = VertexIn[1].vLigthPosition.xyz;
    VertexOut.mProjectionMatrix = VertexIn[1].vProjectionMatrix;
    VertexOut.mVertexWorldSpace = VertexIn[1].vVertexWorldSpace;
    VertexOut.mVertex = gl_in[1].gl_Position;
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

    VertexOut.mBaricentric = vec3(0, 0, 1);
    VertexOut.mTexCoord = vec2(0.0, 1.0);
    VertexOut.mPosition = VertexIn[2].vPosition.xyz;
    VertexOut.mNormal = normal;
    VertexOut.mLigthPosition = VertexIn[2].vLigthPosition.xyz;
    VertexOut.mProjectionMatrix = VertexIn[2].vProjectionMatrix;
    VertexOut.mVertexWorldSpace = VertexIn[2].vVertexWorldSpace;
    VertexOut.mVertex = gl_in[2].gl_Position;
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}
