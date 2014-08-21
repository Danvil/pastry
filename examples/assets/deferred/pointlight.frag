#version 150

uniform vec3 lightpos;
uniform vec3 lightcol;
uniform float lightfalloff;

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texMaterial;

in vec2 uv;
out vec4 outColor;

void main()
{
	vec3 pos = texture(texPosition, uv).xyz;
	vec3 normal = texture(texNormal, uv).xyz;
	vec3 color = texture(texColor, uv).xyz;
	vec3 mat = texture(texMaterial, uv).xyz;
	float roughness = mat.x;
	float metallic = mat.y;
	float specularity = mat.z;

	vec3 d = lightpos - pos;
	float ld = length(d);
	float q = dot(d,normal) / ld / (1.0f + lightfalloff*ld*ld);
	if(q < 0) q = 0;

	vec3 reflectray = reflect(-pos, normal);
	float cosangle = max(dot(d,reflectray),0) / ld / length(pos);
	vec3 specular = specularity * pow(cosangle,5) * lightcol;

	if(length(pos) == 0) {
		outColor = vec4(0,0,0,1);
	}
	else {
		outColor = vec4(q*lightcol*color + specular, 1);
	}
}
