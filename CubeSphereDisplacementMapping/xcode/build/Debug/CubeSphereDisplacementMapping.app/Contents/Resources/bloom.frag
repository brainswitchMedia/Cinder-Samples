#version 400

uniform sampler2D   uSampler;
uniform int         width;
uniform int         height;
uniform bool        horizontal;
uniform float       weight[ 3 ];
uniform float       offset[ 3 ];

in vec2             TexCoord0;

layout (location = 0) out vec4 oColor;

uniform float offset2[3] = float[] (0.0, 1.3846153846, 3.2307692308);
uniform float weight2[3] = float[] (0.2270270270, 0.3162162162, 0.0702702703);


void main()
{             
    
    vec2 tex_offset = vec2( 1.0 / width, 1.0 / height );
    vec4 samplerResult	= texture( uSampler, vec2(TexCoord0.x, TexCoord0.y)).rgba;
    
    vec3 result = samplerResult.rgb * weight[0];
    
    if( horizontal )
    {
        for( int i = 1; i < 3; ++i )
        {
            result += texture( uSampler, ( vec2(TexCoord0) + vec2( tex_offset.x * offset[i], 0.0) ) ).rgb * weight[i];
            result += texture( uSampler, ( vec2(TexCoord0) - vec2( tex_offset.x * offset[i], 0.0) )).rgb * weight[i];
        }
    }
    else
    {
        for( int i = 1; i < 3; ++i )
        {
            result += texture( uSampler, ( vec2(TexCoord0) + vec2(0.0, tex_offset.y * offset[i]) ) ).rgb * weight[i];
            result += texture( uSampler, ( vec2(TexCoord0) - vec2(0.0, tex_offset.y * offset[i]) ) ).rgb * weight[i];
        }
    }
    
    oColor = vec4( result.rgb, 1.0 );
}
