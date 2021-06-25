#version 330 core

uniform sampler2D	uDiffuseMap, uNormalMap, uStripeMap, uSpecular;
uniform vec3		uLightLocViewSpace;
uniform vec2        Time;
uniform float       uColorR;
uniform float       uColorG;
uniform float       uColorB;

uniform float       uColorR2;
uniform float       uColorG2;
uniform float       uColorB2;

uniform float       uSpecularAttenuation;
uniform float       uDiffuseStripesAttenuation;


// inputs passed from the vertex shader
in vec4             VertexViewSpace;
in vec3             NormalViewSpace, TangentViewSpace, BitangentViewSpace;
in vec2             TexCoord0;
in float            distFromCenter;

layout (location = 0) out vec4 renderTrails;
layout (location = 1) out vec4 renderScene;
layout (location = 2) out vec4 renderBloom;

const float ONE_OVER_PI = 1.0 / 3.14159265;

void main()
{
    vec2 texCoord = vec2((TexCoord0.x, TexCoord0.y));
    
    vec3 vMappedNormal = texture(uNormalMap, vec2(TexCoord0.x, TexCoord0.y+ Time.x)).rgb * 2.0 - 1.0;
    vec3 uStripeMap = texture( uStripeMap, vec2(TexCoord0.x, TexCoord0.y+ 2.0*Time.x + Time.y ) ).rgb;
    vec3 uSpecularMap = texture( uSpecular, vec2(TexCoord0.x, TexCoord0.y+ Time.x) ).rgb;
    vec3 uDifuseMap = texture( uDiffuseMap, vec2(TexCoord0.x, TexCoord0.y+ Time.x) ).rgb;

    // modify it using the normal & tangents from the 3D mesh (normal mapping)
	 vec3 normal = normalize(( TangentViewSpace * vMappedNormal.x ) + ( BitangentViewSpace * vMappedNormal.y ) + ( NormalViewSpace * vMappedNormal.z ));

	vec3 vToCamera = normalize( -VertexViewSpace.xyz );
	vec3 light = normalize( VertexViewSpace.xyz );
    vec3 light2 = normalize( uLightLocViewSpace - VertexViewSpace.xyz );

	vec3 reflect = normalize(reflect(light, normal));

	// calculate diffuse term
    vec3 NormalViewSpaceNormalized = normal;
	float fDiffuse = max( dot( light2, NormalViewSpaceNormalized), 0.0 );
    float fDiffuse2 = clamp( fDiffuse, 0.0, 1.0);
    fDiffuse = clamp( fDiffuse, 0.0, 0.8);

	// calculate specular term
	float fSpecularPower = 20.0;
	float fSpecular = pow( max( dot(reflect, -vToCamera), 0.0), fSpecularPower );
    float fSpecular2 = pow( max( dot(reflect, vToCamera), 0.0), fSpecularPower );
	fSpecular = clamp(fSpecular, 0.0, 1.0);
    
    // specular 2
    vec3 viewDir = normalize( vToCamera - VertexViewSpace.xyz );
    float spec = 0.0;
    const float kPi = 3.14159265;
    const float kShininess = 8.0;
    
    const float kEnergyConservation = ( 16.0 + kShininess ) / ( 8.0 * kPi );
    vec3 halfwayDir = normalize( uLightLocViewSpace - viewDir );
    spec = kEnergyConservation * pow( max( dot( normal, halfwayDir ), 0.0), kShininess );
    
    // version 1
    float clampDistFromCenter = clamp( 2.0 - distFromCenter , 0.0, 1.0 );
    vec3 centerColor = clampDistFromCenter * vec3( 1.0, 1.0, 1.0 );
    vec3 vDiffuseColor = vec3( fDiffuse ) * 0.5 * clampDistFromCenter * uDifuseMap * ( 1.0 - uStripeMap )
                       + vec3( 0.1 * fDiffuse ) * ( 1.0 - clampDistFromCenter ) * ( 1.0 - uStripeMap )
                       + uDiffuseStripesAttenuation * ( vec3( uColorR, uColorG, uColorB ) ) * uStripeMap * fDiffuse * ( clampDistFromCenter );

    // Compute curvature
    vec3 n = normalize(NormalViewSpace);

    vec3 dx = dFdx(n);
    vec3 dy = dFdy(n);
    vec3 xneg = n - dx;
    vec3 xpos = n + dx;
    vec3 yneg = n - dy;
    vec3 ypos = n + dy;
    float depth = length(VertexViewSpace);
    float curvature = (cross(xneg, xpos).y - cross(yneg, ypos).x) * 4.0 / depth;
    
    vec3 vSpecularColor = clamp(uSpecularAttenuation * ( (curvature + 0.3) * vec3( uColorR2, uColorG2, uColorB2 ) ) * ( 1.0 - uStripeMap ) * clampDistFromCenter , vec3( 0.0, 0.0, 0.0 ), vec3( 1.0, 1.0, 1.0) );

    renderTrails = vec4(vec3( uColorR, uColorG, uColorB ) * uStripeMap * ( 0.4 + 0.3 * fDiffuse + (( 1.0 - clampDistFromCenter ) * fSpecular * uSpecularMap )), 1.0);

    renderScene = vec4(( vDiffuseColor + vSpecularColor ).rgb, 1.0);
    renderBloom = vec4(vSpecularColor.rgb, 1.0);
}
