#version 150

uniform vec3 lightpos;
uniform vec3 lightcol;
uniform float lightfalloff;

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

	vec3 d = lightpos - pos;
	float ld = length(d);
	float q = dot(d,normal) / ld / (1.0f + lightfalloff*ld*ld);
	if(q < 0) q = 0;
	outColor = vec4(q*lightcol*color,1);
}
