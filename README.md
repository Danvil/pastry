pastry - A C++ toolbox for modern OpenGL
====


Overview
----

pastry is a small C&#43;&#43; toolbox for modern OpenGL 3.x/4.x programming.
Most functionality is provided as light-weight C&#43;&#43; wrappers for commonly used OpenGL objects like shader programs, vertex buffers, frame buffers or textures.
This is *NOT* a complete 3D engine. 

** This version is an early alpha and be no means a sufficiently tested. **

I plan to used this for the [Ludum Dare #48 game competition](http://www.ludumdare.com/compo/).
To the Ludum Dare folks: Feel free to use this and give me feedback :)


Installation
----

Requirements:

* C++11 compatible compiler (e.g. gcc 4.8). Ubuntu: `sudo apt-get install build-essentials`
* OpenGL 3.x compatible graphics card and drivers
* CMake. Ubuntu: `sudo apt-get install cmake`

Dependencies:

* [GLEW](http://glew.sourceforge.net/): Need to download, compile from source and install. Under Ubuntu the package management version can be used: `sudo apt-get install libglew-dev`
* [glfw 3.x](http://www.glfw.org/): Need to download, compile from source and install. Note that Ubuntu package managemer version is tool old (2.x)!
* [SOIL2](https://bitbucket.org/SpartanJ/soil2) (included in the repository)

Ubuntu apt-get line:
`sudo apt-get install libglew-dev

Building:

* `git clone git@bitbucket.org:Danvil/pastry.git`
* `mkdir pastry_build; cd pastry_build`
* `cmake ../pastry -DCMAKE_BUILD_TYPE=Release`
* `make -j2`

Running the examples:

 * For example: `examples/pastry-example-particles`
