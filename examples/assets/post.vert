#version 150

in vec2 pos;
//in float dummy;
out float vdummy;

void main() {
	gl_Position = vec4(pos, 0, 1);
	vdummy = 1.0f;
}
