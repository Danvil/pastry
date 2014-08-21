#version 330

in vec3 TexCoord0;

out vec3 outColor;

uniform samplerCube gCubemapTexture;

void main()
{
	outColor = texture(gCubemapTexture, TexCoord0).xyz;
}
