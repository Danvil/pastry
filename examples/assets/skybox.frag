#version 330

in vec3 TexCoord0;

out vec3 outPosition;
out vec3 outNormal;
out vec3 outColor;
out vec3 final;

uniform samplerCube gCubemapTexture;

void main()
{
	outPosition = TexCoord0;
	outNormal = vec3(0,0,0);
	outColor = texture(gCubemapTexture, TexCoord0).xyz;
	final = outColor;
}
