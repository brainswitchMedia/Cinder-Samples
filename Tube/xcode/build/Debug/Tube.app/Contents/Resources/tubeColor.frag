#version 330 core

uniform sampler2D   uTexGradiant;
uniform sampler2D   uTexStripes;
uniform float       uTime;


in VertexData {
    float   V;
    float   stripeType;
    vec3    normal;
    vec4    position;
    vec2    texCoord;
    
} vVertexIn;


layout (location = 0) out vec4 oColor0;
layout (location = 1) out vec4 oColor1;


void main()
{	    
    float gradiant = texture( uTexGradiant, vVertexIn.texCoord.st ).r;
    vec3 stripes = texture( uTexStripes, vec2( vVertexIn.texCoord.s, vVertexIn.texCoord.t - uTime ) ).rbg;
    
    vec4 finalColor = vec4 ( stripes * gradiant * 0.8, 1.0 );
    vec4 normals = vec4 ( vVertexIn.normal, 1.0 );
    
    oColor0 = vec4( finalColor );
    oColor1 = vec4( normals );

}
