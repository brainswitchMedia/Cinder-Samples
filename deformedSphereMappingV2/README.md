# A Deformed sphere with trails and transformed feedback curl noise particles system

![deformedSphereMapping](https://github.com/brainswitchMedia/Cinder-Samples/blob/master/deformedSphereMappingV2/deformedSphereMappingV2.png)

This code is intended for use with the Cinder C++ library: http://libcinder.org (coded with Cinder v0.9.1)

This Sample is a real-time beat tracker.

This algorithm is based on the following sources:

http://archive.gamedev.net/archive/reference/programming/features/beatdetection/

https://community.sw.siemens.com/s/article/octaves-in-human-hearing

This sample provides a logarithmic repartition of frequencies in 12 subbands and a simple spectrum repartition.
The beat detection is based on a variance calculation.

With this sample it is possible to:

* Adjust the source signal with Gain
    ex:
    Band Gain = 20

* Filter the source signal with a LowShelf Filter if bass frequencies are too loud
    ex:
    LowShelf Frequency = 0.001
    LowShelf Gain = -20

* Smooth the signal in the fft or after the fft
    ex:
    Band Smoothing Factor = 0.1 for soothing in the fft spectrum
    Band Smoothing Energy = 0.1 for soothing after the fft

* Adjust the beat sensitivity detection with a Threshold and a Sensitivity Coeficient
    ex:
    BeatSensitivity Coef = 2 // 1 does not modify the sensitivity and if coef > 1 the sensitivity decreases
    BeatTreshold = 0.15

* Extract up to 4 beats and their energy by selecting different bands of frequencies


The algorithm could be optmized by increasing the number of subbands and/or make the source signal pass through a derivative filter

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
