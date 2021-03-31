#version 330 core

uniform sampler2D   uSamplerObjectsDepth;
uniform float       zFar;
uniform float       zNear;
uniform int         depthMode;

in VertexData{
    vec2    mTexCoord;
    vec4    mColor;
    vec4    mModelViewVertex;
    vec4    mVertex;
    float   mDepth;
} VertexIn;

float lineDepth;

out vec4 oColor;
out vec4 oDepth;

void main(void)
{
    // Calculate line depth
    float zDiff = zFar - zNear;
    lineDepth = 1.0;
    
    // Color
    vec3 color = VertexIn.mColor.rbg;
    
    // Depth calculation in fragment shader, using gl_FragDepth
    if ( depthMode == 1 )
    {
        lineDepth = 0.5 * ( zFar + zNear ) / zDiff + 0.5
        + ( VertexIn.mModelViewVertex.w / VertexIn.mModelViewVertex.z )
        * zFar * zNear / zDiff;
        gl_FragDepth = lineDepth;
    }
    // Depth calculation in vertex shader, using gl_FragDepth
    else if ( depthMode == 2 )
    {
        lineDepth = VertexIn.mDepth;
        gl_FragDepth = lineDepth;
    }
    // Manual Depth calculation here and in composite shader
    if ( depthMode == 3 )
    {
        // Objects depth in screen space
        vec3 ndc = VertexIn.mVertex.xyz / VertexIn.mVertex.w;
        vec2 viewportCoord = ndc.xy * 0.5 + 0.5;
        float objectsDepth = texture( uSamplerObjectsDepth, viewportCoord ).r;
        
        lineDepth = VertexIn.mDepth;
        
        float depthDiff = objectsDepth - lineDepth;
        if( depthDiff < 0.000001 ) discard;  // Discard method
    }
    
    oColor.rgb = color;
    oColor.a = VertexIn.mColor.a;
    oDepth = vec4( lineDepth, lineDepth, lineDepth, 1.0 );
    
}
