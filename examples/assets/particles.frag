#version 150

uniform sampler2D tex;

in vec4 fcolor;
in vec2 fuv;

out vec4 outColor;

void main() {
	outColor = fcolor * texture(tex, fuv);
}
