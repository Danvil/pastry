
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
	static constexpr const char* vertexSource = PASTRY_GLSL(
		in vec2 position;
		in vec3 color;
		out vec3 vcolor;
		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
			vcolor = color;
		}
	);

	static constexpr const char* fragmentSource = PASTRY_GLSL(
		in vec3 vcolor;
		out vec4 outColor;
		void main() {
			outColor = vec4(vcolor, 1.0);
		}
	);

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
		vbo = pastry::array_buffer({
			{"position", GL_FLOAT, 2},
			{"color", GL_FLOAT, 3}
		});
		vbo.init_data(GL_DYNAMIC_DRAW);
		sp = pastry::program(vertexSource, fragmentSource);
		va = pastry::vertex_array(sp, {
			{"position", vbo},
			{"color", vbo}
		});
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

		vbo.update_data(vertices_);
	}

	void render() {
		sp.use();
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
