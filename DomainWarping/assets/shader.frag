#version 330 core

// Simplex Noise

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

uniform sampler2D tex;

// Properties of the particle system
uniform float   xdeformation;
uniform float   ydeformation;
uniform float   zoom;
uniform float   time;
uniform float   noisescale;
uniform float   shade;
uniform float   iTime;

uniform vec3    iResolution;

in vec2         TexCoord;
in vec3         Normal;
in vec3         Position;

layout (location = 0) out vec4 position;
layout (location = 1) out vec4 velocity;


// Uniform data
// From previous state
uniform sampler2D position_sampler_2D;
uniform sampler2D velocity_sampler_2D;


#define  NORMALIZE_GRADIENTS
#undef  USE_CIRCLE
#define COLLAPSE_SORTNET

#define RAMP(r) r >= 1.0f ? 1.0f : (r <= -1.0f ? -1.0f : 15.0f/8.0f * r - 10.0f / 8.0f * pow(r,3) + 3.0f / 8.0f * pow(r,5) )

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// See http://www.iquilezles.org/www/articles/warp/warp.htm for details

// undefine these on old/slow computers

#define SLOW_NORMAL

vec2 hash2( float n )
{
    return fract(sin(vec2(n,n+1.0))*vec2(13.5453123,31.1459123));
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    

    float a = textureLod( tex, ( p + vec2(0.5,0.5))/256.0, 0.0 ).x;
    float b = textureLod( tex, (p + vec2( 1.5,0.5 )) / 256.0, 0.0 ).x;
    float c = textureLod( tex,(p+vec2(0.5,1.5))/256.0,0.0).x;
    float d = textureLod( tex,(p+vec2(1.5,1.5))/256.0,0.0).x;
    return mix( mix( a, b,f.x), mix( c, d,f.x), f.y);
    
    //return a;
    
    
}

//const mat2 mtx = mat2( 0.80,  0.60, -0.60,  0.80 );
const mat2 mtx = mat2( 0.1,  0.613, 0.613, 0.1 );


float fbm4( vec2 p )
{
    float f = 0.0;
    
    f += 0.5000*(-1.0+2.0*noise( p )); p = mtx*p*2.02;
    f += 0.2500*(-1.0+2.0*noise( p )); p = mtx*p*2.03;
    f += 0.1250*(-1.0+2.0*noise( p )); p = mtx*p*2.01;
    f += 0.0625*(-1.0+2.0*noise( p ));
    
    return f/0.9375;
}

float fbm6( vec2 p  )
{
    float f = 0.0;
    
    f += 0.500000*noise( p ); p = mtx*p*2.02;
    f += 0.250000*noise( p ); p = mtx*p*2.03;
    f += 0.125000*noise( p ); p = mtx*p*2.01;
    f += 0.062500*noise( p ); p = mtx*p*2.04;
    f += 0.031250*noise( p ); p = mtx*p*2.01;
    f += 0.015625*noise( p );
    
    return f/0.96875;
}

float func( vec2 q, out vec2 o, out vec2 n )
{
    float ql = length( q );
    q.x += xdeformation*sin(0.11*time+ql*4.0);
    q.y += ydeformation*sin(0.13*time+ql*4.0);
    //q *= 0.7 + 2.0*cos(0.1*zoom*time);
    q *= 0.7 + zoom;

    
    q = (q+1.0)*0.5;
    
    o.x = 0.5 + 0.5*fbm4( vec2(2.0*q*vec2(1.0,1.0)          )  );
    o.y = 0.5 + 0.5*fbm4( vec2(2.0*q*vec2(1.0,1.0)+vec2(5.2))  );
    
    float ol = length( o );
    o.x += 0.02*sin(4.11*noisescale*ol)/ol;
    o.y += 0.2*sin(0.13*noisescale*ol)/ol;
    
    
    n.x = fbm6( vec2(4.0*o*vec2(1.0,1.0)+vec2(9.2))  );
    n.y = fbm6( vec2(4.0*o*vec2(1.0,1.0)+vec2(5.7))  );
    
    vec2 p = 4.0*q + 4.0*n;
    
    float f = 0.5 + 0.5*fbm4( p );
    
    f = mix( f, f*f*f*3.5, f*abs(n.x) );
    
    float g = 0.5+0.5*sin(4.0*p.x)*sin(4.0*p.y);
    f *= 1.0-0.5*pow( g, 8.0 );
    
    return f;
}

float funcs( in vec2 q )
{
    vec2 t1, t2;
    return func(q,t1,t2);
}

vec2 rotateUV(vec2 uv, float rotation, vec2 mid)
{
    return vec2(
                cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) + mid.x,
                cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) + mid.y
                );
}

void main()
{
    /*vec2 uv = gl_FragCoord.xy;
    float angle = iTime;
    float s = sin(angle);
    float c = cos(angle);
    
    mat2 rotationMatrix = mat2( c, s,
                               -s,  c);
    vec2 pivot = vec2( 0.5, 0.5);
    uv = rotationMatrix * (uv - pivot) + pivot;*/

    vec2 of = vec2(0,0);//hash2( float(time)*1113.1 + gl_FragCoord.x + gl_FragCoord.y*119.1 );
    
    // image rotation
    vec2 coord = rotateUV( gl_FragCoord.xy, iTime, vec2(0.5, 0.5) );
    
    vec2 p = gl_FragCoord.xy / iResolution.xy;
    vec2 q = (-iResolution.xy + 2.0 * ( coord + of )) / iResolution.y;
    
    vec2 o, n;
    float f = func(q, o, n);
    vec3 col = vec3(0.0);
    
    
    col = mix( vec3(0.2,0.1,0.4), vec3(0.3,0.05,0.05), f );
    col = mix( col, vec3(0.9,0.9,0.9), dot(n,n) );
    col = mix( col, vec3(0.5,0.2,0.2), 1.0*o.y*o.y );
    
    
    //col = mix( col, vec3(0.0,0.0,0.0), 0.5*smoothstep(1.35,1.8,n.x+n.y) );
    
    col *= f*2.0;
    
    vec2 ex = vec2( 1.0 / iResolution.x, 0.0 );
    vec2 ey = vec2( 0.0, 1.0 / iResolution.y );
    //vec3 nor = normalize( vec3( funcs(q+ex) - f, ex.x, funcs(q+ey) - f ) );
    vec3 nor = normalize( vec3( 2.0*dFdx(f)*iResolution.x, 1.1, 2.0*dFdy(f)*iResolution.y ) );
    nor = cross(nor, nor);

    vec3 lig = normalize( vec3( 0.1, 0.3, -0.1 ) );
    //float dif = clamp( 0.1+dot( nor, lig ), 0.0, 1.0 );
    
    vec3 bdrf;
    //bdrf  = vec3(0.95,0.90,0.35)*(nor.y*0.3+0.5);
    //bdrf += vec3(0.15,0.10,0.05)*dif;
    
    //bdrf  = vec3(0.85,0.90,0.95)*(nor.y*0.5+0.5);
    //bdrf += vec3(0.30,0.25,0.01)*dif*10.0;
    bdrf += vec3(0.30,0.22,0.1)*10.0;

    
    col *= bdrf;
    
    col = vec3(1.0)-col;
    
    col = col*col;
    
    col *= vec3(1.2,1.25,1.5) + vec3(0.01, 0.01, 0.01);
    
    col *= sqrt( p.x * p.y * (1.0-p.x) * (1.0-p.y));
    
    
    vec3 pos = texture( tex, TexCoord ).xyz;
    
    velocity = vec4( col, 1.0 );
    position = vec4( pos, 1.0 );
}




