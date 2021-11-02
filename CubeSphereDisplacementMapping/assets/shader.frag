#version 400

uniform mat3	ciNormalMatrix;
uniform mat4	ciModelView;

uniform samplerCube uReflexionMap, uTranslucencyMap, uOccMap, uHeightMap;

uniform vec3        uLightLocViewSpace;

uniform float       uDisplacement;

layout (location = 0) out vec4 oColor;
layout (location = 1) out vec4 oBloomColor;
layout (location = 2) out vec4 oLightColor;

in vec3     gFacetNormal;
in vec3     gFaceTangent;
in vec3     gFaceBitangent;
in vec4     gVertexViewSpace;
in vec3     gTexCoord3D;
in vec3     gVertexModelSpace;


void main()
{
    vec3 texOcc = texture( uOccMap, gTexCoord3D ).rgb;
    vec3 texTranslucency = texture( uTranslucencyMap, gTexCoord3D ).rgb;
    vec3 texReflexion = texture(uReflexionMap, gTexCoord3D).rgb;
    vec3 heightmap = texture(uHeightMap, gTexCoord3D).rgb;
    
    // calculate view space light vectors
    vec3	vToLight = normalize( uLightLocViewSpace - gVertexViewSpace.xyz);

    vec3 vertexToCamera     = normalize( - gVertexViewSpace.xyz );
    vec3 light              = normalize( uLightLocViewSpace - gVertexViewSpace.xyz );
    
    // conv lin ensemble a vers b
    // a0-a1 -> b0-b1 : 0.0-uDisplacement -> 0.0-1.0
    // out = ((in-a0) / (a1-a0)) x (b1-b0) + b0
    float attenuation = ( length( gVertexModelSpace ) / ( 1.5 *uDisplacement ) );
    float lightAttenuation  = clamp ( pow( attenuation , 1.0 ), 0.0, 1.0 );
    vec3 vTLight = light ;
    float fLTDot = pow( clamp( dot( vertexToCamera, -vTLight ), 0.0, 1.0), 0.55 );
    float texTranslucencyPow = pow( (texTranslucency.r) , 1.0 ) * ( 1.0 - texOcc.r);
    
    vec3 sceneColor =  texOcc.r * vec3( 0.5, 0.5, 0.8 * clamp(abs (attenuation), 0.0, 1.0) );
    vec3 sceneColor1 = vec3( pow( fLTDot * 2.0 * ( texTranslucencyPow ) * lightAttenuation , 2.0)) * sceneColor * texOcc.r;
    vec3 sceneColor2 = vec3( pow( fLTDot * 2.0 * ( texTranslucencyPow ) * lightAttenuation , 2.0)) * sceneColor;

    oColor.rgb = sceneColor1;
    oColor.a = 1.0;

    oBloomColor.rgb = sceneColor2;
    oBloomColor.a = 1.0;

    oLightColor.rgb = vec3(1.0, 0.2, 0.3 ) * pow( texReflexion.r, 3.0 ) * fLTDot;
    oLightColor.a = 1.0;
}
