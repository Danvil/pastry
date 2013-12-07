#include <pastry/pastry.hpp>
#include <map>
#include <string>
#include <cstdio>

#define STB_TRUETYPE_IMPLEMENTATION
#include "dep/stb_truetype.h"

namespace pastry
{

constexpr unsigned int TTF_TEX_SIZE = 512;

struct text_vertex { float x, y, u, v; };

pastry::array_buffer vbo;
pastry::program spo;
pastry::vertex_array vao;

struct font
{
	std::string name;
	std::string filename;
	pastry::texture tex;
	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
};

std::map<std::string, font> g_fonts;

std::string g_default_font;

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
		uniform vec4 color;
		void main() {
			float r = texture(tex, Texcoord).r;
			outColor = color * vec4(r,r,r,r);
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

}

void text_load_font(const std::string& name, const std::string& filename)
{
	font f;
	f.name = name;

	f.filename = filename;
	FILE* fh = std::fopen(filename.data(), "rb");
	if(fh == NULL) {
		std::cerr << "Error loading font '" << filename << "'" << std::endl;
		return;
	}
	char ttf_buffer[1<<20];
	std::fread(ttf_buffer, 1, 1<<20, fh);

	unsigned char temp_bitmap[TTF_TEX_SIZE*TTF_TEX_SIZE];
	stbtt_BakeFontBitmap(
		(unsigned char*)ttf_buffer,0,
		32.0,
		temp_bitmap,TTF_TEX_SIZE,TTF_TEX_SIZE,
		32,96,
		f.cdata); // no guarantee this fits!
	std::fclose(fh);

	f.tex.create();
	f.tex.bind();
	f.tex.width_ = TTF_TEX_SIZE;
	f.tex.height_ = TTF_TEX_SIZE;
	f.tex.channels_ = 1;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TTF_TEX_SIZE,TTF_TEX_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);

	g_fonts[name] = f;

	if(g_default_font.empty()) {
		g_default_font = name;
	}
}

void text_render(float x, float y, const std::string& txt)
{
	text_render(x, y, txt, {1,1,1,1});
}

void text_render(float x, float y, const std::string& txt, const Eigen::Vector4f& rgba)
{
	// find font
	auto it = g_fonts.find(g_default_font);
	if(it == g_fonts.end()) {
		std::cerr << "ERROR text_render: Invalid font '" << g_default_font << "'" << std::endl;
		return;
	}
	font& f = it->second;
	// set projections matrix which has 0/0 top left
	int w, h;
	fb_get_dimensions(w,h);
	spo.get_uniform<Eigen::Matrix4f>("proj").set(
		pastry::math_orthogonal_projection(w, h, -1.0f, +1.0f, true));
	// flip y to provide 0/0 at bottom left for the user
	y = static_cast<float>(h) - y;
	// create one quad per letter/symbol
	const char* text = txt.data();
	std::vector<text_vertex> data;
	while(*text) {
		if(*text >= 32 && *text < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(f.cdata, TTF_TEX_SIZE,TTF_TEX_SIZE, *text-32, &x,&y,&q,1);
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
	// enable transparency
	{	auto state = capability{GL_BLEND,true};
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// update vbo and render data
		spo.use();
		spo.get_uniform<Eigen::Vector4f>("color").set(rgba);
		pastry::texture::activate_unit(0);
		f.tex.bind();
		vao.bind();
		vbo.update_data(data);
		glDrawArrays(GL_TRIANGLES, 0, data.size());
		// disable transparency
	}
}

}
