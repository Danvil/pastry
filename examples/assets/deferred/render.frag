#version 150

in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;
in vec3 Color;
out vec3 outPosition;
out vec3 outNormal;
out vec3 outColor;
out vec3 outMaterial;

void main()
{
	outPosition = Position;
	outNormal = Normal;
	//outColor = mix(texture(texKitten, Texcoord), vec4(Color,1.0), 0.5);
	outColor = Color;
	outMaterial = vec3(0.5, 0.5, 0.5f); // roughness, metallic, specular
}
