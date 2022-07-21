#version 330 core

uniform sampler2D	uTexSpectrum_Mode1;
uniform sampler2D	uTexSpectrum_Mode2;
uniform sampler2D	uTexGradiant_00;
uniform sampler2D	uTexGradiant_01;
uniform sampler2D	uTexGradiant_02;
uniform sampler2D	uTexGradiant_03;
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
uniform int         uMode;


in vec4             vModelPosition;
in vec3				Normal;
in vec2             TexCoord;
in mat4             vProjectionMatrix;
in vec4             vVertexWorldSpace;

const float         PI = 3.14159265359;
const vec3          lightColor = vec3( 1.0, 1.0, 1.0 );
const vec3          albedo = vec3( 1.0, 1.0, 1.0 );
const float         ao = 0.0;

layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


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
    vec3 spectrumCol_Mode1 = texture( uTexSpectrum_Mode1, vec2( uRedCoef * vec3(uSize).r, 0.0 ) ).rgb;
    vec3 spectrumCol_Mode2 = texture( uTexSpectrum_Mode2, vec2( uRedCoef * vec3(uSize).r, 0.0 ) ).rgb;

    // mode 3
    //vec3 spectrumCol = texture( uTexSpectrum, vec2( uRedCoef*vec3(uSize).r, 0.1 ) ).rgb;
    vec3 grad0 = texture( uTexGradiant_00, TexCoord.st ).rgb;
    vec3 grad1 = texture( uTexGradiant_01, TexCoord.st ).rgb;
    vec3 grad2 = texture( uTexGradiant_02, TexCoord.st ).rgb;
    vec3 grad3 = texture( uTexGradiant_03, TexCoord.st ).rgb;    
    
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
    //float translulencyDist = pow( depthFF - dist, 30.0 );
    float translulencyDist2 = pow( depthFF - dist, 40.0 );
    
    
    // PBR
    vec3 N = normalize( Normal );
    vec3 V = normalize( uViewPos - vVertexWorldSpace.xyz );
    vec3 F0 = vec3( 0.04 );
    F0 = mix( F0, albedo, metallic );
    
    vec3 L = normalize( lightPosition - vVertexWorldSpace.xyz );
    vec3 H = normalize( V + L );
    float distance    = length( lightPosition - vVertexWorldSpace.xyz );
    
    // cook-torrance brdf
    float NDF = DistributionGGX( N, H, roughness );
    float G   = GeometrySmith( N, V, L, roughness );
    vec3 F    = fresnelSchlick( max( dot( H, V ), 0.0 ), F0 );
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max( dot( N, V ), 0.0 ) * max( dot( N, L ), 0.0 ) + 0.0001;
    vec3 specular     = numerator / denominator;
    
    if ( uMode == 0 )
    {
        oFragColor0.rgb = grad0.r * spectrumCol_Mode1 * clamp(translulencyDist2, 0, 1 ) + grad2.r * clamp( specular, vec3(0), vec3(1) ) * clamp(translulencyDist2, 0, 1);
        oFragColor1.rgb = grad0.r * spectrumCol_Mode1 * clamp(translulencyDist2, 0, 1 ) ;
        oFragColor2.rgb = grad0.r * spectrumCol_Mode1 * clamp(translulencyDist2, 0, 1 );
    }
    else if ( uMode == 1 )
    {
        oFragColor0.rgb = grad2.r * spectrumCol_Mode1 * clamp( translulencyDist2, 0, 1 ) + grad3.r * clamp( specular, vec3(0), vec3(1) ) * clamp(translulencyDist2, 0, 1);
        oFragColor1.rgb = grad2.r * spectrumCol_Mode1 * clamp( translulencyDist2, 0, 1 ) ;
        oFragColor2.rgb = grad2.r * spectrumCol_Mode1 * clamp( translulencyDist2, 0, 1 );
    }
    else if ( uMode == 2 )
    {
        oFragColor0.rgb = grad2.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 ) + grad2.r * clamp( specular, vec3(0), vec3(1) ) * clamp(translulencyDist2, 0, 1);
        oFragColor1.rgb = grad2.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 ) ;
        oFragColor2.rgb = grad2.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 );
    }
    else
    {
        oFragColor0.rgb = grad1.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 ) + grad1.r * clamp( specular.r, 0, 1 ) *vec3(1.0, 1.0, 1.0)  * clamp(translulencyDist2, 0, 1);
        oFragColor1.rgb = grad1.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 ) ;
        oFragColor2.rgb = grad1.r * spectrumCol_Mode2 * clamp( translulencyDist2, 0, 1 );
    }
    
    oFragColor0.a = 1.0;
    oFragColor1.a = 1.0;
    oFragColor2.a = 1.0;
    
    // Color For capsules big waves
    
    /*oFragColor0.rgb = grad0.r * spectrumCol * clamp(translulencyDist2, 0, 1) + grad2.r * clamp(specular, vec3(0), vec3(1)) * clamp(translulencyDist2, 0, 1);
     oFragColor0.a = 1.0;
     
     oFragColor1.rgb = grad0.r * spectrumCol * clamp(translulencyDist2, 0, 1) ;
     oFragColor1.a = 1.0;
     
     oFragColor2.rgb = grad0.r * spectrumCol * clamp(translulencyDist2, 0, 1);
     oFragColor2.a = 1.0;*/
    
    // Color For capsules Mode 1
    /*oFragColor0.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1) + grad3.r * clamp(specular, vec3(0), vec3(1)) * clamp(translulencyDist2, 0, 1);
    oFragColor0.a = 1.0;
    
    oFragColor1.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1) ;
    oFragColor1.a = 1.0;
    
    oFragColor2.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1);
    oFragColor2.a = 1.0;*/
    
    // Color For capsules mode 2
    /*oFragColor0.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1) + grad2.r * clamp(specular, vec3(0), vec3(1)) * clamp(translulencyDist2, 0, 1);
    oFragColor0.a = 1.0;
    
    oFragColor1.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1) ;
    oFragColor1.a = 1.0;
    
    oFragColor2.rgb = grad2.r * spectrumCol * clamp(translulencyDist2, 0, 1);
    oFragColor2.a = 1.0;*/
    
    // Color For Cones mode 3
    /*oFragColor0.rgb = grad1.r * spectrumCol * clamp(translulencyDist2, 0, 1) + grad1.r * clamp(specular.r, 0, 1) *vec3(1.0, 1.0, 1.0)  * clamp(translulencyDist2, 0, 1);
    oFragColor0.a = 1.0;
    
    oFragColor1.rgb = grad1.r * spectrumCol * clamp(translulencyDist2, 0, 1) ;
    oFragColor1.a = 1.0;
    
    oFragColor2.rgb = grad1.r * spectrumCol * clamp(translulencyDist2, 0, 1);
    oFragColor2.a = 1.0;*/
    
    
}
