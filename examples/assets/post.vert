#version 150

in vec2 pos;

void main() {
	gl_Position = vec4(pos, 0, 1);
}
