
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
		-0.8f, +0.8f, -1.0f, +1.0f,
		+0.8f, +0.8f, +1.0f, +1.0f,
		+0.8f, -0.8f, +1.0f, -1.0f,
		-0.8f, +0.8f, -1.0f, +1.0f,
		+0.8f, -0.8f, +1.0f, -1.0f,
		-0.8f, -0.8f, -1.0f, -1.0f
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
		uniform sampler2D tex1;
		uniform sampler2D tex2;
		void main() {
			vec4 col1 = texture(tex1, Texcoord);
			vec4 col2 = texture(tex2, Texcoord);
			outColor = mix(col1, col2, 1-abs(Texcoord.x*Texcoord.y));
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
			float cr, cg, cb;
			switch((x + y) % 7) {
				case 0: cr=0; cg=0; cb=0; break;
				case 1: cr=1; cg=0; cb=0; break;
				case 2: cr=1; cg=1; cb=0; break;
				case 3: cr=0; cg=1; cb=0; break;
				case 4: cr=0; cg=1; cb=1; break;
				case 5: cr=0; cg=0; cb=1; break;
				case 6: cr=1; cg=1; cb=1; break;
			}
			tex_pixels[i  ] = cr;
			tex_pixels[i+1] = cg;
			tex_pixels[i+2] = cb;
		}
	}
	glActiveTexture(GL_TEXTURE0);
	pastry::texture tex1(160, 120, tex_pixels.data());
	glUniform1i(sp.get_uniform_location("tex1"), 0);

	glActiveTexture(GL_TEXTURE1);
	pastry::texture tex2 = pastry::load_texture("assets/kitten.jpg");
	glUniform1i(sp.get_uniform_location("tex2"), 1);

	pastry::add_renderling(
		[]() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
