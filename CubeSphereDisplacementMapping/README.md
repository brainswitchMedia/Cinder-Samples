# A Deformed sphere with trails and transform feedback curl noise soft particles system

![deformedSphereMapping](https://github.com/brainswitchMedia/Cinder-Samples/blob/master/CubeSphereDisplacementMapping/cube_sphere.jpg)

This code is intended for use with the Cinder C++ library: http://libcinder.org (coded with Cinder v0.9.1)

This Sample is a visual animation that was at the origin just a test for transform feedback and trails. I decided to share it as it is, the code is messy and not optimised. Light shaders are not realistic and certainly not true but it is not intended to reproduce the reality. I removed the audio analysis part.

Here is the video: https://youtu.be/NGrln2cJkeA 

I would say 3 things are ok for use:

* The transform feedback curl noise system that use a geometry shader ( it is a gpu dying soft particles system )  

* The trail part but it should be optimised

* The postprod shader formulas

Everything else is aproximativ and fast coded. 

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
