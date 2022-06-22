#version 330 core

uniform sampler2D	uTexSpectrum;
uniform sampler2D	uTexColor;
uniform sampler2D	uTexGradiant;

uniform vec3		uViewPos;
uniform vec3		uColor;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform float       uSize;
uniform float       uRedCoef;
uniform float		metallic;
uniform float		roughness;
uniform vec3        lightPosition;
uniform float       uColorIndex;
uniform float       uTime;


in vec4             vModelPosition;
in vec3				Normal;
in vec2             TexCoord;
in mat4             vProjectionMatrix;
in vec4             vVertexWorldSpace;

const float         PI = 3.14159265359;
const vec3          lightColor = vec3( 1.0, 0.0, 0.0 );
const vec3          albedo = vec3( 1.0, 1.0, 1.0 );
const float         ao = 0.0;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;


// Based on OpenGL Programming Guide (8th edition), p 457-459.
float checkered( in vec2 uv, in ivec2 freq )
{
    vec2 checker = fract( uv * freq );
    vec2 edge = fwidth( uv ) * freq;
    float mx = max( edge.x, edge.y );
    
    vec2 pattern = smoothstep( vec2(0.5), vec2(0.5) + edge, checker );
    pattern += 1.0 - smoothstep( vec2(0.0), edge, checker );
    
    float factor = pattern.x * pattern.y + ( 1.0 - pattern.x ) * ( 1.0 - pattern.y );
    return mix( factor, 0.5, smoothstep( 0.0, 0.7, mx ) );
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    
    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main()
{
    vec3 spectrumCol = texture( uTexSpectrum, vec2( uRedCoef*vec3(uSize).r, 1 ) ).rgb;
    vec3 Col = texture( uTexColor, vec2( TexCoord.s, TexCoord.t + uTime ) ).rgb;
    vec3 Grad = texture( uTexGradiant, TexCoord.st ).rgb;
    
    
    
    
    //vec3 spectrumCol = texture( uTexSpectrum, vec2( 1.5-uColorIndex, 0.8 ) ).rgb;
    
    
    float DepthLigth = -( uWorldtoLightMatrix * vModelPosition ).z;
    float A = vProjectionMatrix[2].z;
    float B = vProjectionMatrix[3].z;
    float zNear = - B / ( 1.0 - A );
    float zFar = B / ( 1.0 + A );
    float depthFF = 0.5 * ( -A * DepthLigth + B ) / DepthLigth + 0.5;
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // 0 - 200  ->  0 - 1
    float distToLigth = length( uWorldtoLightMatrix * vModelPosition );
    float dist = (( distToLigth - 0.0 ) / ( uDistanceConverstion - 0.0 )) * ( 1.0 - 0.0 ) + 0.0;
    float translulencyDist = pow( depthFF - dist, 30.0 ); // Ok avec le depth
    float translulencyDist2 = pow( depthFF - dist, 40.0 ); // Ok avec le depth
    
    
    // PBR
    vec3 N = normalize( Normal );
    vec3 V = normalize( uViewPos - vVertexWorldSpace.xyz );
    vec3 F0 = vec3( 0.04 );
    F0 = mix( F0, albedo, metallic );
    
    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    // calculate per-light radiance
    
    vec3 L = normalize( lightPosition - vVertexWorldSpace.xyz );
    vec3 H = normalize( V + L );
    float distance    = length( lightPosition - vVertexWorldSpace.xyz );
    float attenuation = 1.0 / ( distance * distance );
    vec3 radiance     = lightColor * attenuation;
    
    // cook-torrance brdf
    float NDF = DistributionGGX( N, H, roughness );
    float G   = GeometrySmith( N, V, L, roughness );
    vec3 F    = fresnelSchlick( max( dot( H, V ), 0.0 ), F0 );
    
    vec3 kS = F;
    vec3 kD = vec3( 1.0 ) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max( dot( N, V ), 0.0 ) * max( dot( N, L ), 0.0 ) + 0.0001;
    vec3 specular     = numerator / denominator;
    
    // add to outgoing radiance Lo
    float NdotL = max(dot( N, L ), 0.0);
    Lo += ( kD * albedo / PI + specular ) * radiance * NdotL;
    //Lo += specular;
    
    // Color palette
    oFragColor0.rgb = uColorIndex * Grad * spectrumCol * clamp(translulencyDist2, 0, 1) +  clamp(specular, vec3(0), vec3(1)) * clamp(translulencyDist2, 0, 1);
    
    // Color uniform
    //oFragColor0.rgb = uColorIndex * Grad * uColor * clamp(translulencyDist2, 0, 1) +  clamp(specular, vec3(0), vec3(1)) * clamp(translulencyDist2, 0, 1);

    oFragColor0.a = 1.0;
    
    oFragColor1.rgb = vec3( 0.0, 0.0, 0.0 );
    oFragColor1.a = 1.0;
    
}
