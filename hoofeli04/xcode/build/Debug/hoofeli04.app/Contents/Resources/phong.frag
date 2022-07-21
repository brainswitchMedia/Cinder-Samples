#version 330 core

uniform sampler2D	uTex0;
uniform int			uTexturingMode;
uniform float       uBrightness;

uniform vec3        uLightPosition;

in VertexData	{
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
    vec4 ligth;
} vVertexIn;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


void main()
{
	// set diffuse and specular colors
	vec3 cDiffuse = vVertexIn.color.rgb;
	vec3 cSpecular = vec3( 0.3 );

	// light properties in view space
	vec3 vLightPosition = vec3( 0.0, 0.0, 0.0 );

	// lighting calculations
	vec3 N = normalize( vVertexIn.normal );
	vec3 L = normalize( vLightPosition - vVertexIn.position.xyz );
	vec3 E = normalize( -vVertexIn.position.xyz );
	vec3 H = normalize( L + E );

	// Calculate coefficients.
	float phong = max( dot( N, L ), 0.0 );

	const float kMaterialShininess = 20.0;
	const float kNormalization = ( kMaterialShininess + 8.0 ) / ( 3.14159265 * 8.0 );
	float blinn = pow( max( dot( N, H ), 0.0 ), kMaterialShininess ) * kNormalization;

	// diffuse coefficient
	vec3 diffuse = vec3( phong );

	if( uTexturingMode == 0 ) {
		//diffuse *= vec3( 1.0, 1.0, 1.0 );
		//diffuse *= 0.5 + 0.5 * checkered( vVertexIn.texCoord, uFreq );
        //diffuse = uBrightness * vec3( 1.0, 1.0, 1.0 ) * distance( vVertexIn.ligth.xyz, vVertexIn.position.xyz );
        diffuse = uBrightness * vec3( 1.0, 1.0, 1.0 ) * clamp( 1.0 -  pow( distance( vVertexIn.ligth.xyz, vVertexIn.position.xyz ), 0.5 ), 0.0, 1.0 );
	}
	else if ( uTexturingMode == 1 )
		diffuse *= texture( uTex0, vVertexIn.texCoord.st ).rgb * pow( distance( vVertexIn.ligth.xyz, vVertexIn.position.xyz ), 5.0 );
        //diffuse *= phong;

	// specular coefficient 
	vec3 specular = blinn * cSpecular;

	// final color
    oFragColor0 = vec4( diffuse , 1.0 );
    oFragColor1 = vec4( 0.0, 0.0, 0.0 , 1.0 );
    oFragColor2 = vec4( diffuse , 1.0 );
}
