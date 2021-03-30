#version 330 core

uniform sampler2D  uObjectsText;
uniform sampler2D  uLineText;
uniform sampler2D  uObjectsDepthText;
uniform sampler2D  uLineDepthText;

uniform float      zFar;
uniform float      zNear;
uniform int        depthMode;

in vec2             TexCoord0;

layout (location = 0) out vec4 oColor;

// LInearize depth for testing (to see depth buffer)
float LinearizeDepth( float depth )
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}


void main( void )
{
    float objectsDepth  = texture( uObjectsDepthText, vec2(TexCoord0.x, TexCoord0.y) ).x;
    float lineDepth     = texture( uLineDepthText, vec2(TexCoord0.x, TexCoord0.y) ).x;
    vec3 objectsText    = texture( uObjectsText, vec2(TexCoord0.x, TexCoord0.y) ).rgb;
    vec3 lineText       = texture( uLineText, vec2(TexCoord0.x, TexCoord0.y) ).rgb;
    
    if ( depthMode == 1 || depthMode == 2 )
    {
        oColor = vec4( objectsText, 1.0);
    }
    if ( depthMode == 3 )
    {
        vec3 objectsColor = objectsText;
        float depthDiff = lineDepth - objectsDepth;
        if( depthDiff < 0.000001 ) objectsColor = vec3(0.0, 0.0, 0.0);
        oColor		= vec4( lineText + objectsColor, 1.0);

        // Depth buffers test
        // Uncomment this lines to visualize depth components
        //float lindepth1 = LinearizeDepth(objectsDepth)/zFar;
        //oColor		= vec4(vec3(lindepth1, lindepth1, lindepth1), 1.0);
        //float lindepth2 = LinearizeDepth(lineDepth)/zFar;
        //oColor		= vec4(vec3(lindepth2, lindepth2, lindepth2), 1.0);
    }
}
