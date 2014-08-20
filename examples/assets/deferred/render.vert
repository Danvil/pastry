#version 150

uniform mat4 proj;
uniform mat4 view;
in vec3 position;
in vec2 texcoord;
in vec3 normal;
out vec3 Position;
out vec2 Texcoord;
out vec3 Normal;
out vec3 Color;

void main()
{
	vec4 pos0 = view*vec4(position, 1.0);
	gl_Position = proj*pos0;
	Position = pos0.xyz;
	Texcoord = texcoord;
	Normal = (view*vec4(normal, 0.0)).xyz;
	Color = vec3(1,0.5,0);
}
