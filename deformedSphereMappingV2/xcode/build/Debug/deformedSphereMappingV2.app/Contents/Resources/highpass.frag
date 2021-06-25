
const float	kExposure	= 6.0;
const float kLuminosity	= 0.333;
const float	kThreshold	= 0.9;
const float	kEdge		= 0.55;

uniform sampler2D	uSampler;
in vec2             TexCoord0;

layout (location = 0) out vec4 oColor;

void main( void )
{
	oColor		= texture( uSampler, vec2(TexCoord0.x, TexCoord0.y));
	oColor.rgb	*= pow( smoothstep( kThreshold, kThreshold + kEdge, dot( oColor.rgb, vec3( kLuminosity ) ) ), kExposure );
}
