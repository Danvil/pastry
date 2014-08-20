#version 150

uniform mat4 view;
in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;
in vec3 Color;
out vec4 outColor;
//uniform sampler2D texKitten;

void main()
{
	vec3 light = (view*vec4(0,3,4,1)).xyz;
	vec3 d = light - Position;
	float ld = length(d);
	float q = dot(d,Normal) / ld / (1.0f + 0.05f*ld*ld);
	if(q < 0) q = 0;
	//vec4 col1 = texture(texKitten, Texcoord);
	//outColor = mix(col1, vec4(Color,1.0), 0.5);
	outColor = vec4(q*Color,1);
}
