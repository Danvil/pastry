
#include <pastry/pastry.hpp>
#include <pastry/../../src/stb_truetype.h>
#include <iostream>

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

void my_stbtt_print(pastry::array_buffer& vbo, float x, float y, char *text)
{
	float scl = 0.005f;
	x /= scl;
	y /= scl;
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
			data.push_back({scl*q.x0,-scl*q.y0,q.s0,q.t0});
			data.push_back({scl*q.x1,-scl*q.y0,q.s1,q.t0});
			data.push_back({scl*q.x1,-scl*q.y1,q.s1,q.t1});
			data.push_back({scl*q.x0,-scl*q.y0,q.s0,q.t0});
			data.push_back({scl*q.x1,-scl*q.y1,q.s1,q.t1});
			data.push_back({scl*q.x0,-scl*q.y1,q.s0,q.t1});
		}
		++text;
	}
	vbo.update_data(data);
	glDrawArrays(GL_TRIANGLES, 0, data.size());
}

int main()
{
	pastry::initialize();

	pastry::array_buffer vbo{
		{"xy", GL_FLOAT, 2},
		{"uv", GL_FLOAT, 2}
	};
	vbo.init_data(GL_DYNAMIC_DRAW);

	std::string vertexSource = PASTRY_GLSL(
		in vec2 position;
		in vec2 texcoord;
		out vec2 Texcoord;
		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
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
	pastry::program spo{vertexSource, fragmentSource};
	spo.use();

	pastry::vertex_array vao{spo, {
		{"position", vbo, "xy"},
		{"texcoord", vbo, "uv"}
	}};
	vao.bind();

	pastry::texture::activate_unit(0);
	spo.get_uniform<int>("tex").set(0);

	char ttf_buffer[1<<20];
	fread(ttf_buffer, 1, 1<<20, fopen("assets/DejaVuSans.ttf", "rb"));
	unsigned char temp_bitmap[512*512];
	stbtt_BakeFontBitmap((unsigned char*)ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

	pastry::texture tex;
	tex.create();
	tex.bind();
	tex.width_ = 512;
	tex.height_ = 512;
	tex.channels_ = 1;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);

	pastry::add_renderling([&vbo]() {
		my_stbtt_print(vbo, -0.5f, 0.0f, "hello world");
	});

	pastry::run();

	return 0;
}
