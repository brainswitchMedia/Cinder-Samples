uniform float   radius;
uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;

in vec4         ciPosition;


void main()
{
    vec4 vertex = ciPosition;
    vertex.xyz	*= radius;
    gl_Position     = ciModelViewProjection * vertex;

}


