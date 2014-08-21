#version 150

in vec2 quv;
out vec2 uv;

void main()
{
	uv = quv;
	gl_Position = vec4(2*quv-1,0,1);
}
