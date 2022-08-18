#version 330 core

uniform vec4    uLightModelView;
uniform float   uBrightness;
uniform float   uPower1;
uniform float   uPower2;


in VertexData	{
    vec4 position;
    vec3 normal;
} vVertexIn;


out vec4 oFragColor;


void main()
{
    
    vec3 direction =  normalize( uLightModelView.xyz - vVertexIn.position.xyz   );
	float dotProduct = max( dot( -vVertexIn.normal, direction ), 0.0 );
    
//    vec3 diffuse =  dotProduct * vec3( uBrightness, uBrightness, uBrightness ) * ( pow( distance( uLightModelView.xyz, vVertexIn.position.xyz ), uPower ) );

    
    vec3 diffuse = vec3( uBrightness, uBrightness, uBrightness ) *  pow( clamp( dotProduct, 0.0, 1.0 ), uPower1 ) * pow( 1.0/distance( uLightModelView.xyz, vVertexIn.position.xyz ), uPower2 );
    
	// final color
	//oFragColor.rgb = uBrightness * vec3( pow( dotProduct, uPower ) );
    oFragColor.rgb = diffuse;

    oFragColor.a = 1.0;
    //oFragColor.a = 0.85;
}
