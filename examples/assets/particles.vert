#version 150

uniform mat4 proj;

in vec2 position;
in float size;
in vec4 color;

out float vsize;
out vec4 vcolor;

void main() {
	gl_Position = proj*vec4(position, 0.0, 1.0);
	vsize = size;
	vcolor = color;
}
