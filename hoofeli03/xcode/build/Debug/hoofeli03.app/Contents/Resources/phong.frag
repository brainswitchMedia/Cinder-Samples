#version 330 core

uniform sampler2D	uTexSpectrum;
uniform sampler2D	uTexAlbedo;
uniform sampler2D	uTexNormal;
uniform sampler2D	uTexRoughness;
uniform sampler2D	uTexLine;
uniform sampler2D	uTextureLigth;

uniform vec3        uLightPosition;
uniform float       uTime;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform float		uEnergy;

in VertexData	{
	vec4 position;
    vec4 worldPos;
	vec3 normal;
	vec2 texCoord;
    mat4 projectionMatrix;
} vVertexIn;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


// Based on OpenGL Programming Guide (8th edition), p 457-459.
float checkered( in vec2 uv, in ivec2 freq )
{
    vec2 checker = fract( uv * freq );
    vec2 edge = fwidth( uv ) * freq;
    float mx = max( edge.x, edge.y );
    
    vec2 pattern = smoothstep( vec2(0.5), vec2(0.5) + edge, checker );
    pattern += 1.0 - smoothstep( vec2(0.0), edge, checker );
    
    float factor = pattern.x * pattern.y + ( 1.0 - pattern.x ) * ( 1.0 - pattern.y );
    return mix( factor, 0.5, smoothstep( 0.0, 0.5, mx ) );
}


vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture( uTexNormal, vec2( 2.0*vVertexIn.texCoord.s, vVertexIn.texCoord.t + uTime ) ).xyz * 2.0 - 1.0;
    
    vec3 Q1  = dFdx( vVertexIn.worldPos.xyz );
    vec3 Q2  = dFdy( vVertexIn.worldPos.xyz );
    vec2 st1 = dFdx( vVertexIn.texCoord );
    vec2 st2 = dFdy( vVertexIn.texCoord );
    
    vec3 N   = normalize( vVertexIn.normal );
    vec3 T  = normalize( Q1 * st2.t - Q2 * st1.t );
    vec3 B  = -normalize( cross( N, T ) );
    mat3 TBN = mat3( T, B, N );
    
    return normalize( TBN * tangentNormal );
}

void main()
{
    float DepthLigth = -( uWorldtoLightMatrix * vVertexIn.worldPos ).z;
    float A = vVertexIn.projectionMatrix[2].z;
    float B = vVertexIn.projectionMatrix[3].z;
    float zNear = - B / ( 1.0 - A );
    float zFar = B / ( 1.0 + A );
    float depthFF = 0.5 * ( -A * DepthLigth + B ) / DepthLigth + 0.5;
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // 0 - 200  ->  0 - 1
    float distToLigth = length( uWorldtoLightMatrix * vVertexIn.worldPos );
    float dist = (( distToLigth - 0.0 ) / ( uDistanceConverstion - 0.0 )) * ( 1.0 - 0.0 ) + 0.0;
    float translulencyDist = pow( depthFF - dist, 10.0 );

	// set diffuse and specular colors
	vec3 cDiffuse = vec3( 1.0, 1.0, 1.0 );
	vec3 cSpecular = vec3( 1.0 );

	// light properties in view space
	vec3 vLightPosition = uLightPosition;

	// lighting calculations
	//vec3 N = normalize( vVertexIn.normal );
    vec3 N = getNormalFromMap();
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
    vec3 spectrumCol = texture( uTexSpectrum, vec2( 0.9+vVertexIn.texCoord.t, 0.1 ) ).rgb;
    vec3 spectrumLigth = texture( uTextureLigth, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t ) ).rgb;

    vec3 albedo = texture( uTexAlbedo, vec2( 2.0*vVertexIn.texCoord.s, vVertexIn.texCoord.t - uTime )).rgb;
    vec3 roughness = texture( uTexRoughness, vec2( 2.0*vVertexIn.texCoord.s, vVertexIn.texCoord.t - uTime )).rgb;
    float line = texture( uTexLine, vVertexIn.texCoord.st ).r;

	// specular coefficient 
	vec3 specular =  cSpecular;

    //diffuse *= 1.0 * checkered( vVertexIn.texCoord, pivec2(0, 300) ) * spectrumCol * albedo;
    diffuse *= line * ( 1.1 * uEnergy * pow( spectrumLigth, vec3( 1.5 ) ) + ( 0.75 * spectrumCol * albedo + 0.015 * blinn * pow( roughness.r, 3 ) ) * clamp( translulencyDist, 0, 1 )) ;
    //diffuse *= line * ( 0.5 * spectrumCol * albedo + 0.015 * blinn * pow( roughness.r, 3 )) * clamp(translulencyDist, 0, 1 );
	// final color
    oFragColor0 = vec4( diffuse, 1.0 );
    oFragColor1 = vec4( 0.0, 0.0, 0.0 , 1.0 );
    oFragColor2 = vec4( 0.0, 0.0, 0.0 , 1.0 );

}
