#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 80) out;

// All that we get from vertex shader

in vec3 vPositionPass[];
in vec3 vVelocityPass[];
in float fLifeTimePass[];
in float fSizePass[];
in float fAgePass[];
in int iTypePass[];

// All that we send further

out vec3 vPositionOut;
out vec3 vVelocityOut;
out float fLifeTimeOut;
out float fSizeOut;
out float fAgeOut;
out int iTypeOut;

// curl uniforms
uniform float Time; // Time
uniform float noisescale;
uniform float length_scale;
//uniform float field_speed;
uniform float noise_strength;
uniform float progression_rate;
uniform float velocityDamping;
uniform float rotational_strength;
uniform float rotational_speed_strength;
uniform float rotational_speed_strength2;
uniform float radiusInSphereSpread;

uniform float centripedAttraction;

uniform vec3 vGenPosition; // Position where new particles are spawned
uniform vec3 vGenVelocityRange;

uniform float fGenSize;

uniform float fGenLifeMin, fGenLifeRange; // Life of new particle - from min to (min+range)
uniform float fTimePassed; // Time passed since last frame

uniform vec3 vRandomSeed; // Seed number for our random number function
vec3 vLocalSeed;

uniform int iNumToGenerate; // How many particles will be generated next time, if greater than zero, particles are generated


// PI
#define PI 3.1415926
#define TWOPI 6.283185306

// Curl
#define F4 0.309016994374947451
vec3 field_main_direction = vec3( 0.0, 1.0, 0.0 );
vec3 pot_directional = vec3(0.0, 0.0, 0.0);

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0; }

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float permute(float x) {
    return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
{
    const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
    vec4 p,s;
    
    p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
    p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
    s = vec4(lessThan(p, vec4(0.0)));
    p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www;
    
    return p;
}

float snoise(vec4 v)
{
    const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                         0.276393202250021,  // 2 * G4
                         0.414589803375032,  // 3 * G4
                         -0.447213595499958); // -1 + 4 * G4
    
    // First corner
    vec4 i  = floor(v + dot(v, vec4(F4)) );
    vec4 x0 = v -   i + dot(i, C.xxxx);
    
    // Other corners
    
    // Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
    vec4 i0;
    vec3 isX = step( x0.yzw, x0.xxx );
    vec3 isYZ = step( x0.zww, x0.yyz );
    //  i0.x = dot( isX, vec3( 1.0 ) );
    i0.x = isX.x + isX.y + isX.z;
    i0.yzw = 1.0 - isX;
    //  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
    i0.y += isYZ.x + isYZ.y;
    i0.zw += 1.0 - isYZ.xy;
    i0.z += isYZ.z;
    i0.w += 1.0 - isYZ.z;
    
    // i0 now contains the unique values 0,1,2,3 in each channel
    vec4 i3 = clamp( i0, 0.0, 1.0 );
    vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
    vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );
    
    //  x0 = x0 - 0.0 + 0.0 * C.xxxx
    //  x1 = x0 - i1  + 1.0 * C.xxxx
    //  x2 = x0 - i2  + 2.0 * C.xxxx
    //  x3 = x0 - i3  + 3.0 * C.xxxx
    //  x4 = x0 - 1.0 + 4.0 * C.xxxx
    vec4 x1 = x0 - i1 + C.xxxx;
    vec4 x2 = x0 - i2 + C.yyyy;
    vec4 x3 = x0 - i3 + C.zzzz;
    vec4 x4 = x0 + C.wwww;
    
    // Permutations
    i = mod289(i);
    float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
    vec4 j1 = permute( permute( permute( permute (
                                                  i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
                                        + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
                               + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
                      + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));
    
    // Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
    // 7*7*6 = 294, which is close to the ring size 17*17 = 289.
    vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;
    
    vec4 p0 = grad4(j0,   ip);
    vec4 p1 = grad4(j1.x, ip);
    vec4 p2 = grad4(j1.y, ip);
    vec4 p3 = grad4(j1.z, ip);
    vec4 p4 = grad4(j1.w, ip);
    
    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    p4 *= taylorInvSqrt(dot(p4,p4));
    
    // Mix contributions from the five corners
    vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
    vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
    m0 = m0 * m0;
    m1 = m1 * m1;
    return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
                   + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;
}


float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x*x*(3 - 2*x);
}


// The vector field potential has three components
vec3 potential(vec3 p, vec3 v)
{
    float L;      // Length scale as described by Bridson
    float speed;  // field speed
    float alpha;  // Alpha as described by Bridson
    float beta;   // amount of curl noise compared to the constant field
    vec3  n;      // Normal of closest surface
    vec3  pot;    // Output potential
    
    L = length_scale;
    beta = noise_strength;
    
    // Start with an empty field
    pot = vec3(0,0,0);
    // Add Noise in each direction
    float progression_constant = 1;
    float time = 0.5*Time;
    pot += L * beta * vec3(
                           snoise(vec4(p.x*noisescale, p.y*noisescale,         p.z*noisescale,        time * progression_rate * progression_constant) / L),
                           snoise(vec4(p.x*noisescale, (p.y + 43)*noisescale,  p.z*noisescale,        time * progression_rate * progression_constant) / L),
                           snoise(vec4(p.x*noisescale, p.y*noisescale,         (p.z + 43)*noisescale, time * progression_rate * progression_constant) / L));
    
    // External directional field
    // Rotational potential gives a constant velocity field
    vec3 p_parallel = dot(vVelocityPass[0], p) * vVelocityPass[0];
    vec3 p_orthogonal = p - p_parallel;
    //vec3 p_orthogonal2 = p - p_parallel;
    
    
    //pot_directional = cross(p_orthogonal, field_main_direction);
    pot_directional = cross(p_orthogonal, vVelocityPass[0]);

    
    //vec3 pot_directional1 = cross(normalize(p), field_main_direction);
    //vec3 pot_directional2 = cross( v, field_main_direction);
    
    // Add the rotational potential
    //pot += (1 - beta) * field_speed * pot_directional;
    
    //pot += rotational_strength * pot_directional + 0.00001*rotational_speed_strength * normalize(pot_directional2);
    pot += rotational_strength * pot_directional;
    
    //pot_directional = cross(p_orthogonal, field_main_direction);
    pot_directional = cross(p_orthogonal, vVelocityPass[0]);
    
    // Affect the field by a sphere
    // The closer to the sphere, the less of the original potential
    // and the more of a tangental potential.
    // The variable d_0 determines the distance to the sphere when the
    // particles start to become affected.
    // float d_0 = L * 0.5;
    // alpha = abs((smoothstep(sphere_radius, sphere_radius + d_0, length(p - sphere_position))));
    // n = normalize(p);
    // pot = (alpha) * pot + (1 - (alpha)) * n * dot(n, pot);
    
    return pot;
}


// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
    
    float fRes =  2.0 - uintBitsToFloat(n);
    
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y * fRes + 415161.0 * fRes, vLocalSeed.z + 324154.0 * fRes);
    return fRes;
}


// This function returns random number from zero to one
vec3 randZeroOneInSphere()
{
    float x;
    float y;
    float z;

    do
    {
        x = 1.0 - 2.0*randZeroOne();
        y = 1.0 - 2.0*randZeroOne();
        z = 1.0 - 2.0*randZeroOne();
    }
    while(1.0 < length(vec3(x, y, z)));

    return vec3(x, y, z);
}


void main()
{
    vLocalSeed = vRandomSeed;
    
    // gl_Position doesn't matter now, as rendering is discarded, so I don't set it at all
    
    fLifeTimeOut = fLifeTimePass[0]-fTimePassed;

    fAgeOut = fAgePass[0];
    //fSizeOut = fSizePass[0];
    fSizeOut = fSizePass[0];
    
    vPositionOut = vPositionPass[0];
    vVelocityOut = vVelocityPass[0];
    
    vec3 normalizedPosition = normalize(vPositionOut);
    
    float epsilon = 0.0001;
    vec3 pot = potential(vPositionOut, vVelocityOut);
    vec3 p = vPositionOut;
    
    // Partial derivatives of different components of the potential
    float dp3_dy = (pot.z - potential(vec3(p.x, p.y + epsilon, p.z), vec3(0.0, 0.0, 0.0))).z / epsilon;
    float dp2_dz = (pot.y - potential(vec3(p.x, p.y, p.z + epsilon), vec3(0.0, 0.0, 0.0))).y / epsilon;
    float dp1_dz = (pot.x - potential(vec3(p.x, p.y, p.z + epsilon), vec3(0.0, 0.0, 0.0))).x / epsilon;
    float dp3_dx = (pot.z - potential(vec3(p.x + epsilon, p.y, p.z), vec3(0.0, 0.0, 0.0))).z / epsilon;
    float dp2_dx = (pot.y - potential(vec3(p.x + epsilon, p.y, p.z), vec3(0.0, 0.0, 0.0))).y / epsilon;
    float dp1_dy = (pot.x - potential(vec3(p.x, p.y + epsilon, p.z), vec3(0.0, 0.0, 0.0))).x / epsilon;
    
    vec3 vel = vec3(dp3_dy - dp2_dz, dp1_dz - dp3_dx, dp2_dx - dp1_dy);

    if( iTypePass[0] != 0 && iTypeOut == 0 )
    {
        vPositionOut +=  vVelocityOut;
        vec3 pot_directional2 = cross( vVelocityOut, vel);
        vec3 pot_directional3 = cross( vel, pot_directional2);

        vVelocityOut = velocityDamping * vVelocityOut + vel + rotational_speed_strength2 * normalize( pot_directional3) + rotational_speed_strength * normalize(pot_directional2) + normalizedPosition*centripedAttraction;
    }

    iTypeOut = iTypePass[0];
    
    if( iTypeOut == 0 )
    {
        EmitVertex();
        EndPrimitive();
        
        for(int i = 0; i < iNumToGenerate; i++)
        {
            
            vec3 randInSphere = 9.0*randZeroOneInSphere();
            
            vVelocityOut = vec3(vGenVelocityRange.x*randInSphere.x, vGenVelocityRange.y*randInSphere.y, vGenVelocityRange.z*randInSphere.z);
            vPositionOut = radiusInSphereSpread * vVelocityOut;

            fLifeTimeOut = fGenLifeMin + fGenLifeRange * randZeroOne();
            vVelocityOut = vVelocityOut * 0.2;
 
            fAgeOut = fLifeTimeOut;
            
            fSizeOut = fGenSize;
            
            iTypeOut = 1;
            EmitVertex();
            EndPrimitive();
        } 
    }
    else if( fLifeTimeOut > 0.0 )
    { 
        EmitVertex(); 
        EndPrimitive();  
    }
}
