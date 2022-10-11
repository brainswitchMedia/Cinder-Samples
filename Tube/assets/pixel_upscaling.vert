#version 330 core

// Superior texture filter for dynamic pixel art upscaling
// https://csantosbh.wordpress.com/2014/02/05/automatically-detecting-the-texture-filter-threshold-for-pixelated-magnifications/

uniform mat4	ciModelViewProjection;
uniform vec2    uTextureSize;

in vec4			ciPosition;
in vec2			ciTexCoord0;

out vec2		TexCoord;
out vec2        vUv;

void main()
{
    float w = uTextureSize.x;
    float h = uTextureSize.y;
    
    vUv = ciTexCoord0 * vec2( w, h );
    
    TexCoord    = ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
}

