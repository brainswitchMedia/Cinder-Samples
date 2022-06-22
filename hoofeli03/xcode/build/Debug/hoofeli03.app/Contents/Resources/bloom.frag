#version 330 core

uniform sampler2D   uTexture;
uniform int         uWidth;
uniform int         uHeight;
uniform bool        uHorizontal;
uniform float       uWeight[ 5 ];

in vec2             TexCoord;

layout (location = 0) out vec4 oFragColor0;

//uniform float weight[5] = float[] ( 0.1170270270, 0.845945946, 0.0816216216, 0.0940540541, 0.0162162162 );

void main()
{             
     //vec2 tex_offset = 1.0 / textureSize( Texture, 0 ); // gets size of single texel
     vec2 tex_offset = vec2( 1.0 / uWidth, 1.0 / uHeight ); // gets size of single texel

     
     vec3 result = texture( uTexture, TexCoord.st ).rbg * uWeight[0];
     //vec3 result = texture2D( Texture, gl_TexCoord[0].st ).rbg * 0.3;

     
    if( uHorizontal )
     {
         for( int i = 1; i < 5; ++i )
         {
            result += texture( uTexture, TexCoord + vec2( tex_offset.x * i, 0.0 ) ).rgb * uWeight[i];
            result += texture( uTexture, TexCoord - vec2( tex_offset.x * i, 0.0 ) ).rgb * uWeight[i];
         }
     }
     else
     {
         for( int i = 1; i < 5; ++i )
         {
             result += texture( uTexture, TexCoord + vec2( 0.0, tex_offset.y * i ) ).rgb * uWeight[i];
             result += texture( uTexture, TexCoord - vec2( 0.0, tex_offset.y * i ) ).rgb * uWeight[i];
         }
     }
     
    oFragColor0 = vec4( result, 1.0 );
}
