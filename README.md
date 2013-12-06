pastry - A C++ toolbox for modern OpenGL
====


Overview
----

pastry is a C\+\+ toolbox for modern OpenGL 3.x/4.x programming.

pastry is aimed at 2D and 2.5D applications - it is not a complete 3D engine. 

** This version is an early alpha and by no means complete or sufficiently tested. **

List of features:

* Light-weight C++ OpenGL wrappers for e.g. shader programs, vertex buffers, textures and textures
* Various OpenGL helpers
* 3D math (thanks to Eigen)
* Texture loading (thanks to stb_image and SOIL2)
* Text rendering (thanks to stb_truetype)
* Sprites (static/animated)
* Particle effects
* Post effects

I plan to use pastry for the [Ludum Dare 48h game competition](http://www.ludumdare.com/compo/).
To the Ludum Dare folks: Feel free to try pastry and give me feedback :)

Runs and compiles well under Ubuntu 13.10 64-bit. Also runs under Windows 7 64-bit using mingw-w64 as cross compiler.


Installation
----

Requirements:

* C++11 compatible compiler (e.g. gcc 4.8).
* OpenGL 3.3 / GLSL 1.5 compatible graphics card or better
* CMake for building from source

Dependencies:

* [GLEW](http://glew.sourceforge.net/)
* [glfw 3.x](http://www.glfw.org/): Need to pull from git, compile from source (`make`) and install (`make install`). Note that Ubuntu (13.10) package manager version is tool old!
* [Eigen 3.x](http://eigen.tuxfamily.org)
* SOIL2 and stb are included in the pastry repository

Ubuntu apt-get line to install them all:

`sudo apt-get install build-essential cmake libglew-dev libeigen3-dev`

Building:

* `git clone git@bitbucket.org:Danvil/pastry.git`
* `mkdir pastry_build; cd pastry_build`
* `cmake ../pastry -DCMAKE_BUILD_TYPE=Release`
* `make -j`

Running the examples:

 * For example: `examples/pastry-example-particles`
 * Be sure to have examples/assets in the right location


TODO
----

* instanced rendering glDrawArraysInstanced
* minimalistic GUI (button, label)
* more 2D/3D camera matrix functions
* mesh loading
