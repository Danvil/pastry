
#include <pastry/pastry.hpp>
#include <pastry/pastry_gl.hpp>
#include <iostream>
#include <cmath>

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	std::cout << "Generating array buffer" << std::endl;

	pastry::array_buffer vbo;
	vbo.id_create();
	float vertices[] = {
		-0.5f,  0.5f,
		+0.5f,  0.5f,
		 0.5f, -0.5f,
		-0.5f,  0.5f,
		 0.5f, -0.5f,
		-0.5f, -0.5f 
	};
	vbo.data(vertices, sizeof(vertices), GL_STATIC_DRAW);

	std::cout << "Compiling vertex shader" << std::endl;

	std::string vertexSource = 
		"#version 150\n"
		"in vec2 position;\n"
		"void main() {\n"
		"	gl_Position = vec4(position, 0.0, 1.0);\n"
		"}\n";
	pastry::vertex_shader sv(vertexSource);

	std::cout << "Compiling fragment shader" << std::endl;

	std::string fragmentSource =
		"#version 150\n"
		"out vec4 outColor;\n"
		"uniform vec3 color;\n"
		"void main() {\n"
		"	outColor = vec4(color, 1.0);\n"
		"}\n";
	pastry::fragment_shader sf(fragmentSource);

	std::cout << "Creating shader program" << std::endl;

	pastry::program sp(sv,sf);
	sp.use();

	std::cout << "Setting vertex attributes" << std::endl;

	pastry::vertex_array va(sp,
		pastry::va<float,2>("position")
	);
	va.bind();

	std::cout << "Adding renderling" << std::endl;

	pastry::add_renderling(
		[&sp]() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		},
		[&sp](float t, float dt) {
			float a = (1.0f + std::sin(t*4.0f))/2.0f;
			sp.get_uniform<float,3>("color").set({a, 0.0f, 1.0f-a});
		});

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
