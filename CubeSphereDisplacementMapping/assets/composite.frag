#version 400

uniform float   exposure;
uniform float   gamma;
uniform float   light;


uniform sampler2D	uSamplerScene;
uniform sampler2D	uSamplerBloom;
uniform sampler2D	uSamplerLight;

in vec2		TexCoord0;

layout ( location = 0 ) out vec4 oColor;

void main( void )
{
    vec3 hdrColor	= texture( uSamplerScene, TexCoord0 ).rgb;
    vec3 bloomColor	= texture( uSamplerBloom, TexCoord0 ).rgb;
    vec3 lightColor	= texture( uSamplerLight, TexCoord0 ).rgb;

    // Additive blending
    //hdrColor += pow(bloomColor, vec3(4.0));
    hdrColor += bloomColor+ clamp( lightColor * light, -0.5, 0.95 );

    // Tone mapping
    vec3 result = vec3( 1.0 ) - exp( -hdrColor * exposure );
    
    // Gamma correct
    result = pow( result, vec3( 1.0 / gamma ) );
    oColor.rgb =  result ;
    oColor.a = 1.0;

}
