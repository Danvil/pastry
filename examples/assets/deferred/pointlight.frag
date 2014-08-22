#version 150

#define PI 3.1415926535897932384626433832795

uniform vec3 lightpos;
uniform vec3 lightcol;
uniform float lightfalloff;

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texMaterial;

in vec2 uv;
out vec4 outColor;

float square(float x) { return x*x; }

float G1(float dot_nv, float k)
{ return dot_nv / (dot_nv*(1.0f-k)+k); }

void main()
{
	vec3 pos = texture(texPosition, uv).xyz;
	vec3 normal = texture(texNormal, uv).xyz;
	vec3 color = texture(texColor, uv).xyz;
	vec3 mat = texture(texMaterial, uv).xyz;
	float roughness = mat.x;
	float metallic = mat.y;
	float specularity = mat.z;
	vec3 F0 = vec3(1.00f,0.71f,0.29f);//vec3(0.04f,0.04f,0.04f);

	vec3 n = normal;
	vec3 l = normalize(lightpos - pos);
	vec3 v = normalize(-pos);
	vec3 h = normalize(l + v); // TODO

	// light strength
	vec3 L = lightcol * max(0.0f, dot(n,l)) / (1.0f + lightfalloff*square(length(lightpos-pos)));

	// Specular D: Disney's GGX/Trowbridge-Reitz
	float alpha = square(roughness);
	float alpha2 = square(alpha);
	float Dh = alpha2 / (PI*square(square(dot(n,h))*(alpha2 - 1.0f) + 1.0f));

	// Specular G: Schlick model, but with k=alpha/2
	float k = square(roughness + 1.0f) / 8.0f;
	float Glvh = G1(dot(n,l), k) * G1(dot(n,v), k);
	// normalization
	float z = 4.0f * dot(n,l) * dot(n,v);

	// Specular F: Schlick’s approximation / Spherical Gaussian approx.
	float dot_vh = dot(v,h);
	vec3 Fvh = F0 + (1.0f - F0)*pow(2.0f, (-5.55473f*dot_vh-6.98316f)*dot_vh);

	vec3 final = (Dh * Fvh * Glvh / z) * L * color;
	if(length(pos) == 0) {
		final = vec3(0,0,0);
	}
	outColor = vec4(final, 1);
}
