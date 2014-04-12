
#include <pastry/pastry.hpp>
#include <pastry/gl.hpp>
#include <iostream>

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	std::cout << "Generating array buffer" << std::endl;

	pastry::array_buffer vbo{
		{"pos", GL_FLOAT, 2},
		{"uv", GL_FLOAT, 2},
		pastry::layout_skip_bytes(4),
		{"color", GL_FLOAT, 3}
	};
	std::vector<float> vertices = {
		-0.8f, +0.8f, -1.0f, +1.0f, 234.0f, 1.0f, 0.0f, 0.0f,
		+0.8f, +0.8f, +1.0f, +1.0f, 234.0f, 1.0f, 1.0f, 0.0f,
		+0.8f, -0.8f, +1.0f, -1.0f, 234.0f, 0.0f, 1.0f, 0.0f,
		-0.8f, +0.8f, -1.0f, +1.0f, 234.0f, 1.0f, 0.0f, 1.0f,
		+0.8f, -0.8f, +1.0f, -1.0f, 234.0f, 0.0f, 1.0f, 1.0f,
		-0.8f, -0.8f, -1.0f, -1.0f, 234.0f, 0.0f, 0.0f, 1.0f
	};
	vbo.init_data(vertices, GL_STATIC_DRAW);

	std::cout << "Compiling vertex shader" << std::endl;

	std::string vertexSource = PASTRY_GLSL(
		in vec2 position;
		in vec2 texcoord;
		in vec3 color;
		out vec2 Texcoord;
		out vec3 Color;
		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
			Texcoord = texcoord;
			Color = color;
		});
	pastry::vertex_shader sv(vertexSource);

	std::cout << "Compiling fragment shader" << std::endl;

	std::string fragmentSource = PASTRY_GLSL(
		in vec2 Texcoord;
		in vec3 Color;
		out vec4 outColor;
		uniform sampler2D texKitten;
		void main() {
			vec4 col1 = texture(texKitten, Texcoord);
			outColor = mix(col1, vec4(Color,1.0), 0.5);
		});
	pastry::fragment_shader sf(fragmentSource);

	std::cout << "Creating shader program" << std::endl;

	pastry::program sp(sv,sf);
	sp.use();

	std::cout << "Setting up vertex arrays" << std::endl;

	pastry::vertex_array va(sp, {
		{"position", vbo, "pos"},
		{"texcoord", vbo, "uv"},
		{"color", vbo}
	});
	va.bind();

	std::cout << "Texture" << std::endl;

	pastry::texture_base::activate_unit(0);
	pastry::texture_base tex = pastry::texture_load("assets/kitten.jpg");
	tex.bind();

	sp.get_uniform<int>("texKitten").set(0);

	pastry::scene_add(
		[]() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
