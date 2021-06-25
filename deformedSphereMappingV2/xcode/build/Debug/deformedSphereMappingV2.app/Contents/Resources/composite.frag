
uniform float	kAttenuation;
uniform float	kAttenuation2;
uniform float   kExposure;
uniform float	kChromaticAberration;

uniform float   kGamma;
uniform float	kTheta;

uniform vec2		uPixel;

uniform sampler2D	uSamplerTextureColor;
uniform sampler2D	uSamplerBloom;
uniform sampler2D	uSamplerTextureTrail;

in vec2             TexCoord0;

layout (location = 0) out vec4 oColor;


void main( void )
{
    vec2 o		= uPixel * 0.01;
    vec4 sum	= vec4( 0.0 );
    float trailChromaticAberration = kChromaticAberration* 1.3;
    sum			+= texture( uSamplerTextureTrail, vec2(TexCoord0.x, TexCoord0.y) + vec2( -o.x,  o.y ) );
    sum			+= texture( uSamplerTextureTrail, vec2(TexCoord0.x, TexCoord0.y) + vec2( -o.x, -o.y ) );
    sum			+= texture( uSamplerTextureTrail, vec2(TexCoord0.x, TexCoord0.y) + vec2(  o.x,  o.y ) );
    sum			+= texture( uSamplerTextureTrail, vec2(TexCoord0.x, TexCoord0.y) + vec2(  o.x, -o.y ) );
    vec4 trailBloom	= sum * kAttenuation;
    
    vec3 colorbloom	= vec3( 0.0 );
    vec3 color = vec3( 0.0 );

    colorbloom.r = texture( uSamplerBloom, vec2( TexCoord0.x + kChromaticAberration,	TexCoord0.y ) ).r;
    colorbloom.g = texture( uSamplerBloom, vec2( TexCoord0.x + 0.0,                     TexCoord0.y ) ).g;
    colorbloom.b = texture( uSamplerBloom, vec2( TexCoord0.x - kChromaticAberration,	TexCoord0.y ) ).b;
        
    color.r = texture( uSamplerTextureColor, vec2( TexCoord0.x + kChromaticAberration,	TexCoord0.y ) ).r;
    color.g = texture( uSamplerTextureColor, vec2( TexCoord0.x + 0.0,                   TexCoord0.y ) ).g;
    color.b = texture( uSamplerTextureColor, vec2( TexCoord0.x - kChromaticAberration,	TexCoord0.y ) ).b;
    
    oColor.rgb		= colorbloom.rgb * kAttenuation2 + color.rgb + kTheta * trailBloom.rgb;
    oColor.rgb		*= kExposure;
    oColor.rgb		= pow( oColor.rgb, vec3( kGamma ) );
    oColor.a = 1.0;

}
