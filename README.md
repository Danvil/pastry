pastry - A C++ toolbox for modern OpenGL
====


Overview
----

pastry is a small C\+\+ toolbox for modern OpenGL 3.x/4.x programming.
Most functionality is provided as light-weight C\+\+ wrappers for commonly used OpenGL objects like shader programs, vertex buffers, frame buffers or textures.
This is *NOT* a complete 3D engine. 

** This version is an early alpha and be no means complete or sufficiently tested. **

I plan to used this for the [Ludum Dare #48 game competition](http://www.ludumdare.com/compo/).
To the Ludum Dare folks: Feel free to use this and give me feedback :)

Runs and compiles well under Ubuntu 13.10. Also runs under Windows 7 64-bit using mingw-w64 as cross compiler.


Installation
----

Requirements:

* C++11 compatible compiler (e.g. gcc 4.8).
* OpenGL 3.x compatible graphics card and properly installed drivers
* CMake

Dependencies:

* [GLEW](http://glew.sourceforge.net/)
* [glfw 3.x](http://www.glfw.org/): Need to pull from git, compile from source (`make`) and install (`make install`). Note that Ubuntu (13.10) package manager version is tool old!
* [SOIL2](https://bitbucket.org/SpartanJ/soil2): Code included in this repository.
* [Eigen 3.x](http://eigen.tuxfamily.org)

Ubuntu apt-get line to install them all:

`sudo apt-get install build-essential cmake libglew-dev libeigen3-dev`

Building:

* `git clone git@bitbucket.org:Danvil/pastry.git`
* `mkdir pastry_build; cd pastry_build`
* `cmake ../pastry -DCMAKE_BUILD_TYPE=Release`
* `make -j2`

Running the examples:

 * For example: `examples/pastry-example-particles`
