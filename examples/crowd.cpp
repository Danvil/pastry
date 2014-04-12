
#include <pastry/pastry.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

constexpr float WORLD_SIZE = 128;

constexpr unsigned NUM_AGENST = 100;

constexpr float SPEED = 5.0f;

struct agent
{
	Eigen::Vector2f position;
	Eigen::Vector2f goal;
	float health;

	agent() {
		position = Eigen::Vector2f::Zero();
		goal = position;
		health = 10.0f;
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
		const float s = 3.0f;
		std::vector<float> v{
			-s,-s, 0, 0,
			-s,+s, 0,+1,
			+s,-s,+1, 0,
			+s,+s,+1,+1
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
				outColor = vec4(0.76,0.92,1,1);
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

int main()
{
	pastry::initialize();

	pastry::scene_add(std::make_shared<agent_group>());

	pastry::run();

	return 0;
}
