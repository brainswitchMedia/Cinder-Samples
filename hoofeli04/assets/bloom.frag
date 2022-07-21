#version 330 core

uniform sampler2D   uTexture;
uniform int         uWidth;
uniform int         uHeight;
uniform int         uHorizontal;
uniform float       uWeight[ 5 ];

in vec2             TexCoord;

layout (location = 0) out vec4 oFragColor0;

//float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             
     //vec2 tex_offset = 1.0 / textureSize( Texture, 0 ); // gets size of single texel
     vec2 tex_offset = vec2( 1.0 / 1024, 1.0 / 1024 ); // gets size of single texel

     vec3 result = texture( uTexture, TexCoord.st ).rbg * uWeight[0];
     //vec3 result = texture2D( Texture, gl_TexCoord[0].st ).rbg * 0.3;

     
    if( uHorizontal == 1 )
     {
         for( int i = 1; i < 5; ++i )
         {
            result += texture( uTexture, TexCoord + vec2( tex_offset.x * i, 0.0 ) ).rgb * uWeight[i];
            result += texture( uTexture, TexCoord - vec2( tex_offset.x * i, 0.0 ) ).rgb * uWeight[i];
         }
     }
     else if ( uHorizontal == 0 )
     {
         for( int i = 1; i < 5; ++i )
         {
             result += texture( uTexture, TexCoord + vec2( 0.0, tex_offset.y * i ) ).rgb * uWeight[i];
             result += texture( uTexture, TexCoord - vec2( 0.0, tex_offset.y * i ) ).rgb * uWeight[i];
         }
     }
     
    oFragColor0 = vec4( result, 1.0 );
}
