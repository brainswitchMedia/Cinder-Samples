#version 330 core

uniform mat4    ciModelViewProjection;
uniform mat4    ciModelView;

in vec4         ciPosition;
in vec4         ciColor;

uniform float   zFar;
uniform float   zNear;

out VertexData{
	vec4        mColor;
    vec4        mModelViewVertex;
    vec4        mVertex;
    float       mDepth;
} VertexOut;


void main(void)
{
	VertexOut.mColor = ciColor;
	gl_Position = ciModelViewProjection * ciPosition;
    vec4 vertex = ciModelView * ciPosition;
    VertexOut.mModelViewVertex = vertex;
    VertexOut.mVertex = gl_Position;
    
    // Calculate Depth in vertex shader
    float zDiff = zFar - zNear;
    VertexOut.mDepth = 0.5 * ( zFar + zNear ) / zDiff + 0.5
                    + ( vertex.w / vertex.z )
                    * zFar * zNear / zDiff;
}
