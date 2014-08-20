#version 150

in vec3 uv;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texColor;

out vec4 outColor;

void main()
{
	vec3 pos = texture(texPosition, uv);
	vec3 normal = texture(texNormal, uv);
	vec3 color = texture(texColor, uv);
	vec3 light = (view*vec4(0,3,4,1)).xyz;
	vec3 d = light - Position;
	float ld = length(d);
	float q = dot(d,Normal) / ld / (1.0f + 0.05f*ld*ld);
	if(q < 0) q = 0;
	//vec4 col1 = texture(texKitten, Texcoord);
	//outColor = mix(col1, vec4(Color,1.0), 0.5);
	outColor = vec4(q*Color,1);
}
