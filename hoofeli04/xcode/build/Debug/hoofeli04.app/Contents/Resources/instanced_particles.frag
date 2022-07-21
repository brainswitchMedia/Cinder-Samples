#version 330 core

uniform sampler2D	uTex0;
uniform mat4		uWorldtoLightMatrix;
uniform float		uDistanceConverstion;
uniform float       uPower;
uniform float       uBrightness;
uniform int         uMode;
uniform float       uTime;
uniform float       uPowerTranslulencyDist;

in VertexData	{
    vec4 position;
    vec3 normal;
    vec2 texCoord;
    vec4 light;
    mat4 projectionMatrix;
    vec4 vertexWorldSpace;
} vVertexIn;


layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;
layout (location = 2) out vec4 oFragColor2;


void main()
{
    float DepthLigth = -( uWorldtoLightMatrix * vVertexIn.vertexWorldSpace ).z;
    float A = vVertexIn.projectionMatrix[2].z;
    float B = vVertexIn.projectionMatrix[3].z;
    float zNear = - B / ( 1.0 - A );
    float zFar = B / ( 1.0 + A );
    float depthFF = 0.5 * ( -A * DepthLigth + B ) / DepthLigth + 0.5;
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-2.0 -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    // 0 - 200  ->  0 - 1
    float distToLigth = length( uWorldtoLightMatrix * vVertexIn.vertexWorldSpace );
    float dist = (( distToLigth - 0.0 ) / ( uDistanceConverstion - 0.0 )) * ( 1.0 - 0.0 ) + 0.0;
    float translulencyDist = pow( depthFF - dist, uPowerTranslulencyDist );
    
    // set diffuse and specular colors
    vec3 cDiffuse = vec3( 1.0, 1.0, 1.0 );
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
    
    if( uMode == 0 ) {
        //diffuse *= vec3( 1.0, 1.0, 1.0 );
        //diffuse *= 0.5 + 0.5 * checkered( vVertexIn.texCoord, uFreq );
        //diffuse = uBrightness * vec3( 1.0, 1.0, 1.0 ) * distance( vVertexIn.ligth.xyz, vVertexIn.position.xyz );
        //diffuse = uBrightness * vec3( 1.0, 1.0, 1.0 ) * clamp( 1.0 -  pow( distance( vVertexIn.light.xyz, vVertexIn.position.xyz ), uPower ), 0.0, 1.0 );// * clamp(translulencyDist2, 0, 1);
        diffuse = uBrightness * vec3( 1.0, 1.0, 1.0 ) * clamp( pow( distance( vVertexIn.light.xyz, vVertexIn.position.xyz ), uPower ), 0.0, 1.0 ) * translulencyDist;// * clamp(translulencyDist2, 0, 1);

    }
    else if ( uMode == 1 )
        diffuse = uBrightness * texture( uTex0, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t + uTime ) ).rgb * pow( distance( vVertexIn.light.xyz, vVertexIn.position.xyz ), uPower ) * translulencyDist;// * clamp(translulencyDist2, 0, 1);
        //diffuse = uBrightness * texture( uTex0, vVertexIn.texCoord.st ).rgb;// * clamp(translulencyDist2, 0, 1);

    //diffuse *= phong;
    
    // specular coefficient
    vec3 specular = blinn * cSpecular;
    
    
    // final color
    oFragColor0 = vec4( diffuse , 1.0 );
    oFragColor1 = vec4( 0.0, 0.0, 0.0 , 1.0 );
    oFragColor2 = vec4( diffuse , 1.0 );
}
