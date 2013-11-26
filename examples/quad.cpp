
#include <pastry/pastry.hpp>
#include <pastry/pastry_gl.hpp>
#include <iostream>

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
		"void main() {\n"
		"	outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n";
	pastry::fragment_shader sf(fragmentSource);

	std::cout << "Creating shader program" << std::endl;

	pastry::program sp(sv,sf);
	sp.use();

	std::cout << "glGetAttribLocation" << std::endl;

	pastry::vertex_attribute va = sp.get_attribute("position");
	va.configure(2, GL_FLOAT, GL_FALSE, 0, 0);
	va.enable();

	// std::cout << "glBindVertexArray" << std::endl;

	// GLuint vao;
	// glGenVertexArrays(1, &vao);
	// glBindVertexArray(vao);

	pastry::add_renderling(
		[]() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
