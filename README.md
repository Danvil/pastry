gl.hpp
====

A C\+\+ toolbox for modern OpenGL 3.x/4.x programming.

This single header file provides light-weight wrappers for OpenGL functionalities. They make using OpenGL easier but do not require the overhead of a full 3D engine.

Currently provided:
* shaders: Simplifies in-source writing, loading, compiling and linking of OpenGL shaders
* uniforms: Use Eigen types, std::vector and std::array to set and get uniforms
* textures: Load images into OpenGL textures
* buffer objects: Manage buffer objects which can for example hold vertex data.

I plan to use gl.hpp for the [Ludum Dare 48h game competition](http://www.ludumdare.com/compo/).
To the Ludum Dare folks: Feel free to try pastry and give me feedback :)

Runs and compiles well under Ubuntu 14.04 64-bit. Also runs under Windows 7 64-bit using mingw-w64 as cross compiler.

Installation
----

Just copy gl.hpp to your include directory.

Requirements:

* C++11 compatible compiler (e.g. gcc 4.8).
* OpenGL 3.3 / GLSL 1.5 compatible graphics card or better

License
----

Copyright (c) 2014 David Weikersdorfer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

