# A Cube Sphere ( Spherified Cube ) tessellated and deformed with a cubemap texture

![deformedSphereMapping](https://github.com/brainswitchMedia/Cinder-Samples/blob/master/CubeSphereDisplacementMapping/cube_sphere.jpg)

This code is intended for use with the Cinder C++ library: http://libcinder.org (coded with Cinder v0.9.1)

Demo videos:
https://youtu.be/IT7WLxrKGlQ
https://vimeo.com/637163233/20feb47578

This Sample contains:
* A spherisation source code in the CubeSphere.cpp file
* A tessellation shader  
* A geometry shader to deform the sphere with a cubemap heightmap
* A fragment shader for illumination ( which has nothing to do with physics principles )
* A bloom shader 
* An anti-aliasing fxaa shader

Notes:
I could not calculate normals in the geometry shader with the cubemap projection. I should use 6 2d textures insted of cube map lookups but I had no time to try it
A good link explaining this issue: https://gamedev.stackexchange.com/questions/66642/tangent-on-generated-sphere

The tessellation can be manually modified:
KEY_LEFT : mInnerLevel--
KEY_RIGHT : mInnerLevel++
KEY_DOWN : mOuterLevel--
KEY_UP : mOuterLevel++

The height factor can be also manually modified:
KEY_m : + 0.05
KEY_n : - 0.05

Links cubesphere:
https://arm-software.github.io/opengl-es-sdk-for-android/tessellation.html
https://www.iquilezles.org/www/articles/patchedsphere/patchedsphere.htm
http://www.songho.ca/opengl/gl_sphere.html#example_cubesphere

links tessellation:
https://ogldev.org/www/tutorial30/tutorial30.html
https://stackoverflow.com/questions/24166446/glsl-tessellation-displacement-mapping
https://community.khronos.org/t/computing-the-tangent-space-in-the-fragment-shader/52861

Link bloom:
https://learnopengl.com/Advanced-Lighting/Bloom

----------------------------------------------------------------------------------

Copyright (c) 2021, brainswitchMedia. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
