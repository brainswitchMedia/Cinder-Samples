#version 330 core

uniform sampler2D   uTexFirstPass;
uniform float       uExposure;
uniform float       uDecay;
uniform float       uDensity;
uniform float       uWeight;
uniform vec2        uLightPositionOnScreen;

in vec2             TexCoord;

const int           NUM_SAMPLES = 100 ;

float               weights[NUM_SAMPLES];
vec4                tempcolor = vec4( 0.0, 0.0, 0.0, 0.0 );
vec4                outcolor = vec4( 0.0, 0.0, 0.0, 0.0 );
vec4                gradiantColor;

layout (location = 0) out vec4 oFragColor0;


void main()
{
   float illuminationDecay = 1.0;
 
    for( int i=0; i < NUM_SAMPLES ; i++ )
    {
        weights[i] = illuminationDecay * log( uWeight );
        //weights[i] = 1.0/illuminationDecay * uWeight;

        illuminationDecay *= uDecay;
    }

    vec2 deltaTextCoord = vec2( TexCoord.st - uLightPositionOnScreen.xy );
    vec2 textCoo = TexCoord.st;
    deltaTextCoord *= 1.0 / float( NUM_SAMPLES ) * uDensity;

    for( int i=0; i < NUM_SAMPLES ; i++ )
    {
        textCoo -= deltaTextCoord;
        vec4 sample = texture( uTexFirstPass, textCoo );
        sample.rbg *= weights[i];
        
        sample.rbg = 0.2 * sample.rbg;
        sample.rbg *= weights[i];
        outcolor += sample;

    }
    
    vec3 test = texture( uTexFirstPass, deltaTextCoord ).rbg;

    
    //oFragColor0.rgb = outcolor.rgb * uExposure;
    //oFragColor0.rgb = test;
    oFragColor0.rgb = vec3( 1.0 ) - exp( -outcolor.rgb * uExposure );

    oFragColor0.a = 1.0;

    
   
 
}

				
