#version 150

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

out vec2 fuv;

void main() {
	float R = 1.0f;
	gl_Position = gl_in[0].gl_Position + vec4(-R,-R,0,0);
	fuv = vec2(0,0);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(-R,+R,0,0);
	fuv = vec2(0,1);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(+R,-R,0,0);
	fuv = vec2(1,0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(+R,+R,0,0);
	fuv = vec2(1,1);
	EmitVertex();
	
	EndPrimitive();
}
