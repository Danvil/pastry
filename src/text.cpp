#include <pastry/pastry.hpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace pastry
{

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

pastry::array_buffer vbo;
pastry::program spo;
pastry::vertex_array vao;
pastry::texture tex;

void initialize_text()
{
	vbo = pastry::array_buffer{
		{"xy", GL_FLOAT, 2},
		{"uv", GL_FLOAT, 2}
	};
	vbo.init_data(GL_DYNAMIC_DRAW);

	std::string vertexSource = PASTRY_GLSL(
		in vec2 position;
		in vec2 texcoord;
		out vec2 Texcoord;
		uniform mat4 proj;
		void main() {
			gl_Position = proj * vec4(position, 0.0, 1.0);
			Texcoord = texcoord;
		}
	);
	std::string fragmentSource = PASTRY_GLSL(
		in vec2 Texcoord;
		out vec4 outColor;
		uniform sampler2D tex;
		void main() {
			float r = texture(tex, Texcoord).r;
			outColor = vec4(r,r,r,1.0f);
		}
	);
	spo = pastry::program{vertexSource, fragmentSource};
	spo.use();

	vao = pastry::vertex_array{spo, {
		{"position", vbo, "xy"},
		{"texcoord", vbo, "uv"}
	}};
	vao.bind();

	spo.get_uniform<int>("tex").set(0);

	char ttf_buffer[1<<20];
	fread(ttf_buffer, 1, 1<<20, fopen("assets/DejaVuSans.ttf", "rb"));
	unsigned char temp_bitmap[512*512];
	stbtt_BakeFontBitmap((unsigned char*)ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

	tex.create();
	tex.bind();
	tex.width_ = 512;
	tex.height_ = 512;
	tex.channels_ = 1;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
}

void text_render(float x, float y, const std::string& txt)
{
	Eigen::Matrix4f proj = math_orthogonal_projection(512.0f, 512.0f, -1.0f, +1.0f);
	spo.get_uniform<Eigen::Matrix4f>("proj").set(proj);

	spo.use();
	pastry::texture::activate_unit(0);
	tex.bind();
	vao.bind();
	const char* text = txt.data();
	struct vertex { float x, y, u, v; };
	std::vector<vertex> data;
	while(*text) {
		if(*text >= 32 && *text < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);
			// std::cout << q.x0 << " " << q.y0 << " " << q.s0 << " " << q.t1 << std::endl;
			// std::cout << q.x1 << " " << q.y0 << " " << q.s1 << " " << q.t1 << std::endl;
			// std::cout << q.x1 << " " << q.y1 << " " << q.s1 << " " << q.t0 << std::endl;
			// std::cout << q.x0 << " " << q.y1 << " " << q.s0 << " " << q.t0 << std::endl;
			// data.push_back({0.01f*q.x0,0.01f*q.y0,q.s0,q.t1});
			// data.push_back({0.01f*q.x1,0.01f*q.y0,q.s1,q.t1});
			// data.push_back({0.01f*q.x1,0.01f*q.y1,q.s1,q.t0});
			// data.push_back({0.01f*q.x0,0.01f*q.y1,q.s0,q.t0});
			data.push_back({q.x0,q.y0,q.s0,q.t0});
			data.push_back({q.x1,q.y0,q.s1,q.t0});
			data.push_back({q.x1,q.y1,q.s1,q.t1});
			data.push_back({q.x0,q.y0,q.s0,q.t0});
			data.push_back({q.x1,q.y1,q.s1,q.t1});
			data.push_back({q.x0,q.y1,q.s0,q.t1});
		}
		++text;
	}
	vbo.update_data(data);
	glDrawArrays(GL_TRIANGLES, 0, data.size());
}

}
