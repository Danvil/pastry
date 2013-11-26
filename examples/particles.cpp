
#include <pastry/pastry.hpp>
#include <pastry/pastry_gl.hpp>
#include <iostream>
#include <cmath>

class particle_effect
: public pastry::renderling
{
public:
	struct particle
	{
		float x, y;
		float cr, cg, cb;
	};
	struct vertex
	{
		float x, y;
		float cr, cg, cb;
	};
private:
	const char* vertexSource = 
		"#version 150\n"
		"in vec2 position;\n"
		"in vec3 color;\n"
		"out vec3 vcolor;\n"
		"void main() {\n"
		"	gl_Position = vec4(position, 0.0, 1.0);\n"
		"	vcolor = color;\n"
		"}\n";

	const char* fragmentSource =
		"#version 150\n"
		"in vec3 vcolor;\n"
		"out vec4 outColor;\n"
		"void main() {\n"
		"	outColor = vec4(vcolor, 1.0);\n"
		"}\n";

	pastry::array_buffer vbo;
	pastry::program sp;
	pastry::vertex_array va;

	std::vector<particle> particles_;
	std::vector<vertex> vertices_;

	float radius_, size_, speed_;
	float color_r_, color_g_, color_b_;

public:
	particle_effect(float radius, float size, float speed, float color_r, float color_g, float color_b) {
		radius_ = radius;
		size_ = size;
		speed_ = speed;
		color_r_ = color_r;
		color_g_ = color_g;
		color_b_ = color_b;
		vbo.id_create();
		vbo.bind();
		pastry::vertex_shader sv = pastry::vertex_shader(vertexSource);
		pastry::fragment_shader sf = pastry::fragment_shader(fragmentSource);
		sp = pastry::program(sv, sf);
		sp.use();
		va.id_create();
		va.bind();
		pastry::vertex_attribute va1 = sp.get_attribute("position");
		va1.configure(2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
		va1.enable();
		pastry::vertex_attribute va2 = sp.get_attribute("color");
		va2.configure(3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
		va2.enable();
	}

	void update(float t, float dt) {
		constexpr int num = 100;
		float a = radius_*std::sin(t * speed_);

		particles_.clear();
		for(int i=0; i<num; i++) {
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(num);
			particle p;
			p.x = a*std::cos(phi);
			p.y = a*std::sin(phi);
			p.cr = color_r_;
			p.cg = color_g_;
			p.cb = color_b_;
			particles_.push_back(p);
		}

		vertices_.clear();
		vertices_.reserve(particles_.size()*6);
		for(const particle& p : particles_) {
			vertices_.push_back(vertex{p.x-size_, p.y+size_, p.cr, p.cg, p.cb});
			vertices_.push_back(vertex{p.x+size_, p.y+size_, p.cr, p.cg, p.cb});
			vertices_.push_back(vertex{p.x+size_, p.y-size_, p.cr, p.cg, p.cb});
			vertices_.push_back(vertex{p.x-size_, p.y+size_, p.cr, p.cg, p.cb});
			vertices_.push_back(vertex{p.x+size_, p.y-size_, p.cr, p.cg, p.cb});
			vertices_.push_back(vertex{p.x-size_, p.y-size_, p.cr, p.cg, p.cb});
		}

		vbo.bind();
		vbo.data(vertices_.data(), vertices_.size()*sizeof(vertex), GL_DYNAMIC_DRAW);
	}

	void render() {
		sp.use();
		vbo.bind();
		va.bind();
		glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
	}
};

int main()
{
	pastry::initialize();

	pastry::add_renderling(std::make_shared<particle_effect>(
		0.7f, 0.03f, 2.0f, 1.0f, 1.0f, 0.0f));
	pastry::add_renderling(std::make_shared<particle_effect>(
		0.5f, 0.02f, 3.0f, 1.0f, 0.0f, 0.0f));
	pastry::add_renderling(std::make_shared<particle_effect>(
		0.3f, 0.01f, 4.0f, 0.0f, 0.5f, 1.0f));

	pastry::run();

	return 0;
}
