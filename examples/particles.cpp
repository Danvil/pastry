
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
		float vx, vy;
		float s;
		float cr, cg, cb, ca;
	};

private:
	pastry::array_buffer vbo;
	pastry::program spo;
	pastry::vertex_array va;
	pastry::texture tex;

	std::vector<particle> particles_;

	float radius_, size_, spring_;
	float color_r_, color_g_, color_b_, color_a_;

	static constexpr int NUM = 1000;

public:
	particle_effect(float radius, float size, float spring, float color_r, float color_g, float color_b, float color_a) {
		radius_ = radius;
		size_ = size;
		spring_ = spring;
		color_r_ = color_r;
		color_g_ = color_g;
		color_b_ = color_b;
		color_a_ = color_a;
		// shader source
		std::string vertexSource = PASTRY_GLSL(
			uniform mat4 proj;
			in vec2 position;
			in float size;
			in vec4 color;
			out float vsize;
			out vec4 vcolor;
			void main() {
				gl_Position = proj*vec4(position, 0.0, 1.0);
				vsize = size;
				vcolor = color;
			}
		);
		std::string geometrySource = PASTRY_GLSL(
			layout(points) in;
			layout(triangle_strip) out;
		//		layout(line_strip) out;
			layout(max_vertices = 4) out;
			in float vsize[];
			in vec4 vcolor[];
			out vec4 fcolor;
			out vec2 fuv;
			void main() {
				float s = vsize[0];
				fcolor = vcolor[0];

				gl_Position = gl_in[0].gl_Position + vec4(-s,-s,0,0);
				fuv = vec2(0,0);
				EmitVertex();
				
				gl_Position = gl_in[0].gl_Position + vec4(-s,+s,0,0);
				fuv = vec2(0,1);
				EmitVertex();
				
				gl_Position = gl_in[0].gl_Position + vec4(+s,-s,0,0);
				fuv = vec2(1,0);
				EmitVertex();

				gl_Position = gl_in[0].gl_Position + vec4(+s,+s,0,0);
				fuv = vec2(1,1);
				EmitVertex();
				
				EndPrimitive();
			}
		);
		std::string fragmentSource = PASTRY_GLSL(
			in vec4 fcolor;
			in vec2 fuv;
			out vec4 outColor;
			uniform sampler2D tex;
			void main() {
				outColor = fcolor * texture(tex, fuv);
			}
		);
		// create render objects
		vbo = pastry::array_buffer({
			{"position", GL_FLOAT, 2},
			pastry::layout_skip<float,2>(),
			{"size", GL_FLOAT, 1},
			{"color", GL_FLOAT, 4}
		});
		vbo.init_data(GL_DYNAMIC_DRAW);
		spo = pastry::program(vertexSource, geometrySource, fragmentSource);
		va = pastry::vertex_array(spo, {
			{"position", vbo},
			{"size", vbo},
			{"color", vbo}
		});
		tex = pastry::texture_load("assets/particle.png");
		// create particles
		particles_.clear();
		for(int i=0; i<NUM; i++) {
			float q = static_cast<float>(i) / static_cast<float>(NUM);
			float phi = 2.0f * 3.14159265359f * q;
			particle p;
			float dev = 1.0f + q;
			p.x = dev*radius_*std::cos(phi);
			p.y = dev*radius_*std::sin(phi);
			p.vx = -p.x;
			p.vy = -p.y;
			p.s = size_;
			p.cr = color_r_;
			p.cg = color_g_;
			p.cb = color_b_;
			p.ca = color_a_;
			particles_.push_back(p);
		}
	}

	void update(float t, float dt) {
		for(particle& p : particles_) {
			float r = std::sqrt(p.x*p.x + p.y*p.y);
			float f = spring_*(radius_ - r);
			p.vx += f*dt*p.x/r;
			p.vy += f*dt*p.y/r;
			p.x += p.vx*dt;
			p.y += p.vy*dt;
		}
		vbo.update_data(particles_);
	}

	void render() {
		// update projection matrix
		int w, h;
		pastry::fb_get_dimensions(w,h);
		constexpr float S = 2.0f;
		float aspect = static_cast<float>(h)/static_cast<float>(w);
		Eigen::Matrix4f proj = pastry::math_orthogonal_projection(
			-S, +S,
			-S*aspect, +S*aspect,
			-1.0f, +1.0f);
		spo.get_uniform<Eigen::Matrix4f>("proj").set(proj);
		// additive blending
		auto state = pastry::capability(GL_BLEND,true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		// render
		pastry::texture::activate_unit(0);
		tex.bind();
		spo.use();
		va.bind();
		glDrawArrays(GL_POINTS, 0, particles_.size());
	}
};

int main()
{
	pastry::initialize();

	pastry::scene_add(std::make_shared<particle_effect>(
		0.7f, 0.15f, 1.0f, 1.0f, 0.0f, 0.0f, 0.1f));
	pastry::scene_add(std::make_shared<particle_effect>(
		0.5f, 0.07f, 1.0f, 0.0f, 1.0f, 0.0f, 0.2f));
	pastry::scene_add(std::make_shared<particle_effect>(
		0.3f, 0.03f, 1.0f, 0.0f, 0.0f, 1.0f, 0.3f));

	pastry::run();

	return 0;
}
