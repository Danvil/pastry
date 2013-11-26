
#include <pastry/pastry.hpp>
#include <pastry/pastry_gl.hpp>
#include <iostream>

#define GLSL(src) "#version 150\n" #src

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	std::cout << "Generating array buffer" << std::endl;

	pastry::array_buffer vbo;
	vbo.id_create();
	float vertices[] = {
		-0.5f, +0.5f, -1.0f, +1.0f,
		+0.5f, +0.5f, +1.0f, +1.0f,
		+0.5f, -0.5f, +1.0f, -1.0f,
		-0.5f, +0.5f, -1.0f, +1.0f,
		+0.5f, -0.5f, +1.0f, -1.0f,
		-0.5f, -0.5f, -1.0f, -1.0f
	};
	vbo.data(vertices, sizeof(vertices), GL_STATIC_DRAW);

	std::cout << "Compiling vertex shader" << std::endl;

	std::string vertexSource = GLSL(
		in vec2 position;
		in vec2 texcoord;
		out vec2 Texcoord;
		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
			Texcoord = texcoord;
		});
	pastry::vertex_shader sv(vertexSource);

	std::cout << "Compiling fragment shader" << std::endl;

	std::string fragmentSource =GLSL(
		in vec2 Texcoord;
		out vec4 outColor;
		uniform sampler2D tex;
		void main() {
			outColor = texture(tex, Texcoord);
		});
	pastry::fragment_shader sf(fragmentSource);

	std::cout << "Creating shader program" << std::endl;

	pastry::program sp(sv,sf);
	sp.use();

	std::cout << "glGetAttribLocation" << std::endl;

	pastry::vertex_attribute va1 = sp.get_attribute("position");
	va1.configure(2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	va1.enable();

	pastry::vertex_attribute va2 = sp.get_attribute("texcoord");
	va2.configure(2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	va2.enable();

	std::cout << "Texture" << std::endl;

	int tex_w = 160;
	int tex_h = 120;
	std::vector<float> tex_pixels(3*tex_w*tex_h);
	for(int y=0,i=0; y<tex_h; y++) {
		for(int x=0; x<tex_w; x++,i+=3) {
			tex_pixels[i  ] = static_cast<float>(x % 256) / 255.0f;
			tex_pixels[i+1] = static_cast<float>(y % 256) / 255.0f;
			tex_pixels[i+2] = static_cast<float>((x*y) % 256) / 255.0f;
		}
	}
	pastry::texture tex(160, 120, tex_pixels.data());

	pastry::add_renderling(
		[]() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}