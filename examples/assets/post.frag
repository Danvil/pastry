#version 150

uniform sampler2D tex;

in vec2 fuv;

out vec4 outColor;

void main() {
	outColor = vec4(1,0,0,1) * texture(tex, fuv);
}
