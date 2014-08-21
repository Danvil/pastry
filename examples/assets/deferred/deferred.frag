#version 150

uniform mat4 view;
in vec2 uv;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texColor;
out vec4 outColor;

void main()
{
	vec3 pos = texture(texPosition, uv).xyz;
	vec3 normal = texture(texNormal, uv).xyz;
	vec3 color = texture(texColor, uv).xyz;

	vec3 light = (view*vec4(0,3,4,1)).xyz;
	vec3 d = light - pos;
	float ld = length(d);
	float q = dot(d,normal) / ld / (1.0f + 0.05f*ld*ld);
	if(q < 0) q = 0;
	outColor = vec4(q*color,1);
}
