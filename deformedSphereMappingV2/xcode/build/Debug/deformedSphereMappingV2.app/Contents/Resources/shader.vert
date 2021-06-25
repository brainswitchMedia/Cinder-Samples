#version 150

uniform mat4	ciModelView;
uniform mat4    ciViewMatrix;
uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;

in vec4			ciPosition;
in vec3			ciNormal;
in vec3			ciTangent;
in vec2			ciTexCoord0;

out vec4		VertexViewSpace;
out vec3		NormalViewSpace;
out vec3		TangentViewSpace;
out vec3		BitangentViewSpace;
out vec2		TexCoord0;
out float       distFromCenter;

// instancied model matrix
in mat4         vInstanceModelMatrix;
in mat3         vInstanceNormalMatrix;

void main()
{
    vec4 p = vInstanceModelMatrix * ciPosition;
	// calculate view space position (required for lighting)
    VertexViewSpace = ciModelView * p;

	// calculate view space normal (required for lighting & normal mapping)
    NormalViewSpace = normalize(vInstanceNormalMatrix * ciNormal);

	// calculate tangent and construct the bitangent (required for normal mapping)
    vec4 t = vInstanceModelMatrix * vec4(ciTangent, 1.0);
	TangentViewSpace = normalize( mat3(vInstanceNormalMatrix) * vec3(t) );

	BitangentViewSpace = normalize( cross( TangentViewSpace, NormalViewSpace ) );

	// pass texture coordinates
	TexCoord0 = ciTexCoord0;

	// vertex shader must always pass projection space position
	 gl_Position = ciModelViewProjection * p;
    
    // Vertex distance from center
    distFromCenter = distance(vec3(0.0, 0.0, 0.0), ciPosition.xyz);

}
