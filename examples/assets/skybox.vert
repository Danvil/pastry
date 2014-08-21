#version 330

in vec3 Position;

uniform mat4 gWVP;

out vec3 TexCoord0;

void main()
{
    vec4 WVP_Pos = gWVP * vec4(Position, 1.0);
    gl_Position = WVP_Pos.xyww; // trick!
    TexCoord0 = Position; // trick!
}
