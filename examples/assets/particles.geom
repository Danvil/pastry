#version 150

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in float vsize[];
in vec4 vcolor[];

out vec4 fcolor;
out vec2 fuv;

void main() {
	float s = vsize[0];
	fcolor = vcolor[0];

	gl_Position = gl_in[0].gl_Position + vec4(-s,-s,0,0);
	fuv = vec2(0,0);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(-s,+s,0,0);
	fuv = vec2(0,1);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(+s,-s,0,0);
	fuv = vec2(1,0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(+s,+s,0,0);
	fuv = vec2(1,1);
	EmitVertex();
	
	EndPrimitive();
}
