# GPU Draw Perfect Lines in 3D with depth information

![3dline](https://github.com/brainswitchMedia/Cinder-Samples/blob/master/GPU3DLinesWithDepth/3dline.png)

This code is intended for use with the Cinder C++ library: http://libcinder.org
  
This code is heavily based on Paul's GeometryShader Sample:
=> https://github.com/paulhoux/Cinder-Samples/tree/master/GeometryShader
 
Depth calculation is based on opengl depthbuffer documentation:
https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
 
and Sergejs Kovrovs's article:
https://gist.github.com/kovrov/a26227aeadde77b78092b8a962bd1a91

This sample shows 2 depth calculation methods with diferent rendering results. 
* The first use gl_FragDepth and the Z-Buffer.
* In the other the depth calculation is done manualy.   

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
