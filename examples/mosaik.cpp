
#include <pastry/pastry.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

constexpr float WORLD_SIZE = 128;

constexpr unsigned NUM_AGENST = 10;

constexpr float SPEED = 5.0f;

constexpr float AGENT_SIZE = 20.0f;

constexpr int PATCH_SIZE = 8;

struct agent
{
	Eigen::Vector2f position;
	Eigen::Vector2f goal;

	agent() {
		position = Eigen::Vector2f::Zero();
		goal = position;
	}

	void update(float t, float dt) {
		Eigen::Vector2f dist = position - goal;
		float dist_norm = dist.norm();
		if(dist_norm < 1) {
			set_random_goal();
			return;
		}
		position += dt*dist*SPEED/dist_norm;
	}

	void set_random_goal() {
		std::uniform_real_distribution<float> r_pos(-WORLD_SIZE, +WORLD_SIZE);
		goal = { r_pos(s_rnd), r_pos(s_rnd) };
	}
};

class agent_group : public pastry::renderling
{
	pastry::array_buffer vbo_mesh;
	pastry::array_buffer vbo_inst;
	pastry::program spo;
	pastry::vertex_array va;

	std::vector<agent> agents_;

public:
	agent_group() {
		// create render objects
		vbo_mesh = pastry::array_buffer({
			{"pos", GL_FLOAT, 2},
			{"uv", GL_FLOAT, 2}
		});
		std::vector<float> v{
			-AGENT_SIZE,-AGENT_SIZE, 0, 0,
			-AGENT_SIZE,+AGENT_SIZE, 0,+1,
			+AGENT_SIZE,-AGENT_SIZE,+1, 0,
			+AGENT_SIZE,+AGENT_SIZE,+1,+1
		};
		vbo_mesh.init_data(v, GL_DYNAMIC_DRAW);

		vbo_inst = pastry::array_buffer({
			{"pos", GL_FLOAT, 2}
		});
		vbo_inst.init_data(GL_DYNAMIC_DRAW);

		std::string vert_src = PASTRY_GLSL(
			uniform mat4 proj;
			uniform mat4 view;
			in vec2 pos;
			in vec2 inst_pos;
			void main() {
				gl_Position = proj*view*vec4(inst_pos+pos, 0.0, 1.0f);
			}
		);
		std::string frag_src = PASTRY_GLSL(
			out vec4 outColor;
			void main() {
				outColor = vec4(0.92,0.76,0,1);
			}
		);
		spo = pastry::program(vert_src, frag_src);

		va = pastry::vertex_array(spo, {
			{"pos", vbo_mesh},
			{"inst_pos", vbo_inst, "pos", 1}
		});

		// create data
		// suns
		for(int i=0; i<NUM_AGENST; i++) {
			agent a;
			std::uniform_real_distribution<float> r_pos(-WORLD_SIZE, +WORLD_SIZE);
			a.position = { r_pos(s_rnd), r_pos(s_rnd) };
			agents_.push_back(a);
		}
	}
	
	void update(float t, float dt) {
		std::vector<float> a_inst_dat;
		a_inst_dat.reserve(agents_.size() * 2);
		for(agent& a : agents_) {
			a.update(t,dt);
			a_inst_dat.push_back(a.position[0]);
			a_inst_dat.push_back(a.position[1]);
		}
		vbo_inst.update_data(a_inst_dat);
	}
	
	void render() {
		// update projection matrix
		spo.get_uniform<Eigen::Matrix4f>("proj").set(
			pastry::math_orthogonal_projection(2.0f*WORLD_SIZE, -1.0f, +1.0f));
		spo.get_uniform<Eigen::Matrix4f>("view").set(
			Eigen::Matrix4f::Identity());
		// render instances
		spo.use();
		va.bind();
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, agents_.size());
	}
};

class render_to_texture : public pastry::renderling
{
	unsigned width_, height_;
	pastry::texture_2d tex_;
	pastry::renderbuffer rbo;
	pastry::framebuffer fbo;

public:
	render_to_texture(unsigned w, unsigned h) {
		tex_.create(GL_NEAREST,GL_CLAMP_TO_EDGE);

		rbo.bind();

		fbo.bind();
		fbo.attach(GL_COLOR_ATTACHMENT0, tex_);
		fbo.attach(GL_DEPTH_ATTACHMENT, rbo);
		fbo.unbind();

		resize(w,h);
	}

	const pastry::texture_2d& tex() {
		return tex_;
	}

	void resize(unsigned w, unsigned h) {
		width_ = w;
		height_ = h;

		fbo.bind();
		tex_.bind();
		tex_.set_image<unsigned char, 4>(GL_RGBA8, width_, height_, 0);
		rbo.bind();
		rbo.storage(GL_DEPTH_COMPONENT16, width_, height_);
		fbo.unbind();
	}

	void update(float t, float dt) {
		if(pastry::key_is_pressed('A')) {
			pastry::texture_save(tex_, "tex.png");
		}
		glViewport(0,0,width_,height_);
		fbo.bind();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void render() {
	}
};

class mosaik : public pastry::renderling
{
private:
	pastry::program spo;
	pastry::array_buffer vbo;
	pastry::vertex_array vao;

	pastry::texture_2d tex_mask_;

	pastry::texture_2d tex_;

public:
	mosaik(const pastry::texture_2d& tex)
	: tex_(tex)
	{
		tex_mask_ = pastry::texture_load("assets/mask.png");
		tex_mask_.set_filter(GL_LINEAR);
		tex_mask_.set_wrap(GL_REPEAT);

		vbo = pastry::array_buffer({
			{"pos", GL_FLOAT, 2}
		});
		vbo.init_data(std::vector<float>{0.0f,0.0f}, GL_STATIC_DRAW);

		std::string src_vert = PASTRY_GLSL(
			in vec2 pos;
			void main() {
				gl_Position = vec4(pos, 0, 1);
			}
		);
		std::string src_geom = PASTRY_GLSL(
			layout(points) in;
			layout(triangle_strip) out;
			layout(max_vertices = 4) out;
			out vec2 postfx_fuv;
			void main() {
				float R = 1.0f;
				gl_Position = gl_in[0].gl_Position + vec4(-R,-R,0,0);
				postfx_fuv = vec2(0,0);
				EmitVertex();	
				gl_Position = gl_in[0].gl_Position + vec4(-R,+R,0,0);
				postfx_fuv = vec2(0,1);
				EmitVertex();	
				gl_Position = gl_in[0].gl_Position + vec4(+R,-R,0,0);
				postfx_fuv = vec2(1,0);
				EmitVertex();
				gl_Position = gl_in[0].gl_Position + vec4(+R,+R,0,0);
				postfx_fuv = vec2(1,1);
				EmitVertex();	
				EndPrimitive();
			}
		);
		std::string src_frag = PASTRY_GLSL(
			uniform sampler2D postfx_tex;
			uniform sampler2D mask;
			uniform int src_width;
			in vec2 postfx_fuv;
			out vec4 postfx_outColor;
			void main() {
				vec4 color =
				 texture(mask, postfx_fuv * src_width)
				 * texture(postfx_tex, postfx_fuv);
				postfx_outColor = color;
			}
		);

		//spo = pastry::load_program("assets/post"); // FIXME
		spo = pastry::program(src_vert, src_geom, src_frag);
		spo.get_uniform<int>("postfx_tex").set(0);
		spo.get_uniform<int>("mask").set(1);
		spo.get_uniform<int>("src_width").set(tex_.width());

		vao = pastry::vertex_array(spo, {{"pos", vbo}});
	}

	void update(float t, float dt) {
	}

	void render() {
		// now render to screen
		pastry::framebuffer::unbind();
		int w, h;
		pastry::fb_get_dimensions(w,h);
		glViewport(0,0,w,h);
		// render quad with texture
		spo.use();
		pastry::texture_2d::activate_unit(0);
		tex_.bind();
		pastry::texture_2d::activate_unit(1);
		tex_mask_.bind();
		vbo.bind();
		vao.bind();
		glDrawArrays(GL_POINTS, 0, 1);
	}
};

int main()
{
	pastry::initialize();

	pastry::scene_add(std::make_shared<agent_group>());

	{
		int w, h;
		pastry::fb_get_dimensions(w,h);
		w = (w / PATCH_SIZE);
		h = (h / PATCH_SIZE);
		auto rtt = std::make_shared<render_to_texture>(w,h);
		pastry::scene_add(rtt, 100000);

		pastry::scene_add(std::make_shared<mosaik>(rtt->tex()), 100001);
	}

	pastry::run();

	return 0;
}
