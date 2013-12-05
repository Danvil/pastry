#version 150

in float dummy;
out float vdummy;

void main() {
	gl_Position = vec4(0, 0, 0, dummy);
	vdummy = dummy;
}
