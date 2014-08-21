#version 150

uniform vec3 lightpos;
uniform vec3 lightcol;
uniform float lightfalloff;

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texMaterial;
uniform samplerCube gCubemapTexture;

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
	float specular = mat.z;

	if(length(pos) == 0) {
		outColor = vec4(0,0,0,1);
	}
	else {
		vec3 reflectray = reflect(-pos, normal);
		vec3 reflcolor = texture(gCubemapTexture, reflectray).xyz;

		outColor = vec4(roughness*color*reflcolor, 1);
	}
}
