#version 330 core


uniform sampler2D   ParticleTex;
uniform sampler2D   ObjectDepthTex;
uniform float       zNear;
uniform float       zFar;
uniform vec2        InvViewportSize; // 1.0/width, 1.0/height
uniform float       Luminosity;
uniform float       BloomLuminosity;

in float            fTransp;
in vec2             texCoord;
in vec4             vertexPosition;

layout (location = 1) out vec4 renderScene;
layout (location = 2) out vec4 renderBloom;

// if color fade
vec3 color = vec3( 1.0-fTransp, 1.0-fTransp, 1.0-fTransp );

// LInearize depth for testing (to see depth buffer)
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

// Linearize depth for calculation
float calc_depth( in float z )
{
    return ( 2.0 * zNear ) / ( zFar + zNear - z * ( zFar - zNear ) );
}


void main() {

    vec4 particleCol      = texture( ParticleTex, vec2(texCoord.x, texCoord.y ) );
    
    // Objects depth in screen space
    vec2 uv = gl_FragCoord.xy * InvViewportSize;
    float objectsDepth = texture( ObjectDepthTex, uv ).r;

    // Soft particles
    float geometryZ = calc_depth( objectsDepth );       // lineriarize other objects depth
    float sceneZ = calc_depth( gl_FragCoord.z );        // lineriarize particles depth
    float a = clamp( geometryZ - sceneZ, 0.0, 1.0 );    // linear clamped diff between scene and particle depth
    float b = smoothstep( 0.0, 0.05, a );                // apply smoothstep to make soft transition
    float alpha = clamp( (1.4 - geometryZ) * ( 1.3 - fTransp ), 0.0, 0.9 ) * particleCol.a  ;
    
    float dynamicColor = clamp( alpha * b * Luminosity, 0.0, 1.0 );
    renderScene = vec4( particleCol.rgb * dynamicColor , dynamicColor );
    renderBloom = vec4( particleCol.rgb * BloomLuminosity * particleCol.a,  BloomLuminosity * particleCol.a );
}
