
#include <pastry/pastry.hpp>
#include <pastry/std.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

constexpr unsigned NUM_STARS = 4000;
constexpr unsigned NUM_STARS_PASSIVE = 20000;
constexpr float WORLD_SIZE = 512;

constexpr unsigned NUM_SUNS = 3;
constexpr float SUNS[NUM_SUNS][2] = {
	{0.11f*WORLD_SIZE, 0.03f*WORLD_SIZE},
	{0.67f*WORLD_SIZE, 0.66f*WORLD_SIZE},
	{-0.54f*WORLD_SIZE, -0.42f*WORLD_SIZE}
};
constexpr float RAD_SUN = 20.0f;

constexpr float RAD_MIN = 2.0f;
constexpr float RAD_MAX = 10.0f;

constexpr float PASSIVE_RAD_MIN = 1.0f;
constexpr float PASSIVE_RAD_MAX = 2.0f;

constexpr float VEL_INIT_SCL = 3.0f;
constexpr float POS_INIT_RAD = 0.65f*WORLD_SIZE;
constexpr float POS_INIT_THICK = 0.35f*POS_INIT_RAD;

constexpr float R_SKIP = 0.01f;
constexpr float G = 5.0f;
constexpr float DENSITY = 1.0f;
constexpr float A_MIN = 0.1f;

constexpr unsigned GRID_DIM = 128;
constexpr float GRID_1 = -640.0f;
constexpr float GRID_2 = +640.0f;

typedef pastry::spatial_grid_2d<std::size_t,GRID_DIM> grid;

struct star
{
	float px, py;
	float pox, poy;
	float ax, ay;
	float r;
	float m;
	unsigned is_active;

	star(unsigned isa) {
		reset();
		is_active = isa;
	}

	void reset() {
		// position / vel / accel
		std::uniform_real_distribution<float> r_pos(-WORLD_SIZE, +WORLD_SIZE);
		px = r_pos(s_rnd);
		py = r_pos(s_rnd);
		pox = px;
		poy = py;
		ax = 0.0f;
		ay = 0.0f;
		// radius / mass
		//std::uniform_real_distribution<float> r_rad(RAD_MIN, RAD_MAX);
		//set_radius(r_rad(s_rnd));
		if(is_active) {
			std::uniform_real_distribution<float> r_q(0, 1);
			float q = r_q(s_rnd);
			float r = RAD_MIN + (RAD_MAX - RAD_MIN)*std::pow(q,16);
			set_radius(r);
		}
		else {
			std::uniform_real_distribution<float> r_q(PASSIVE_RAD_MIN, PASSIVE_RAD_MAX);
			set_radius(r_q(s_rnd));
		}
	}

	void set_radius(float nr) {
		r = nr;
		m = DENSITY*r*r;
	}

	void update(float t, float dt, std::size_t me, std::vector<star>& other, const grid& g);
};

void gravity(const star& p, const star& q, float& df0x, float& df0y) {
	float dx = q.px - p.px;
	float dy = q.py - p.py;
	float r = std::sqrt(dx*dx + dy*dy);
	float rmin = p.r + q.r;
	if(r < rmin)
		r = rmin;
	float f0 = G / (r*r*r);
	df0x = f0*dx;
	df0y = f0*dy;
}

void star::update(float t, float dt, std::size_t me, std::vector<star>& other, const grid& g)
{
	const float R_SEARCH = std::sqrt(G * m / A_MIN);
	if(is_active) {
		g.traverse(px, py, R_SEARCH,
			[this,me,&other](std::size_t k) {
				if(k != me) {
					star& q = other[k];
					if(m > q.m) { // avoid double update // FIXME what if equal?
						float df0x, df0y;
						gravity(*this, q, df0x, df0y);
						ax += df0x * q.m;
						ay += df0y * q.m;
						q.ax -= df0x * m;
						q.ay -= df0y * m;
					}
				}
			});
	}
	{
		float r = px*px + py*py;
		if(r > 10.0f*WORLD_SIZE*WORLD_SIZE) {
			reset();
		}
	}
}


class galaxy : public pastry::renderling
{
	pastry::array_buffer vbo_mesh;
	pastry::array_buffer vbo_inst;
	pastry::program spo;
	pastry::vertex_array va;
	pastry::texture_2d tex;

	std::vector<star> stars_;

public:
	galaxy() {
		// create render objects
		vbo_mesh = pastry::array_buffer({
			{"pos", GL_FLOAT, 2},
			{"uv", GL_FLOAT, 2}
		});
		const float s = 1.0f;
		std::vector<float> v{
			-s,-s, 0, 0,
			-s,+s, 0,+1,
			+s,-s,+1, 0,
			+s,+s,+1,+1
		};
		vbo_mesh.init_data(v, GL_DYNAMIC_DRAW);

		vbo_inst = pastry::array_buffer({
			{"pos", GL_FLOAT, 2},
			{"pos_old", GL_FLOAT, 2},
			{"acc", GL_FLOAT, 2},
			{"rad", GL_FLOAT, 1},
			{"mass", GL_FLOAT, 1},
			{"act", GL_UNSIGNED_INT, 1}
		});
		vbo_inst.init_data(GL_DYNAMIC_DRAW);

		std::string vert_src = PASTRY_GLSL(
			uniform mat4 proj;
			uniform mat4 view;
			in vec2 pos;
			in vec2 uv;
			in vec2 inst_pos;
			in float inst_rad;
			out vec2 vuv;
			void main() {
				gl_Position = proj*view*vec4(inst_pos+inst_rad*pos, 0.0, 1.0f);
				vuv = uv;
			}
		);
		std::string frag_src = PASTRY_GLSL(
			in vec2 vuv;
			uniform sampler2D tex;
			out vec4 outColor;
			void main() {
				outColor = vec4(1,0.92,0.76,1) * texture(tex, vuv);
			}
		);
		spo = pastry::program(vert_src, frag_src);

		va = pastry::vertex_array(spo, {
			{"pos", vbo_mesh},
			{"uv", vbo_mesh},
			{"inst_pos", vbo_inst, "pos", 1},
			{"inst_rad", vbo_inst, "rad", 1}
		});

		tex = pastry::texture_load("assets/particle.png");

		// create data
		// suns
		for(unsigned i=0; i<NUM_SUNS; i++) {
			star s{1};
			s.px = SUNS[i][0];
			s.py = SUNS[i][1];
			s.pox = s.px;
			s.poy = s.py;
			s.set_radius(RAD_SUN);
			stars_.push_back(s);
		}
		// normal
		std::uniform_real_distribution<float> r_vel(-1.0f, +1.0f);
		for(unsigned i=NUM_SUNS; i<NUM_STARS; i++) {
			star s{1};
			// float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM_STARS);
			// s.px = POS_INIT_RAD*std::cos(phi) + POS_INIT_THICK*r_vel(s_rnd);
			// s.py = POS_INIT_RAD*std::sin(phi) + POS_INIT_THICK*r_vel(s_rnd);
			stars_.push_back(s);
		}
		// passive
		for(unsigned i=0; i<NUM_STARS_PASSIVE; i++) {
			star s{0};
			stars_.push_back(s);
		}
	}
	
	void update(float t, float dt) {
		grid g(GRID_1, GRID_1, GRID_2, GRID_2);
		for(std::size_t k=0; k<stars_.size(); k++) {
			g.add(stars_[k].px, stars_[k].py, k);
		}
		for(std::size_t k=0; k<stars_.size(); k++) {
			stars_[k].update(t, dt, k, stars_, g);
		}
		dt = 1.0f / 60.0f;
		float dt2 = dt*dt;
		for(star& p : stars_) {
			float dx = p.px - p.pox;
			float dy = p.py - p.poy;
			p.pox = p.px;
			p.poy = p.py;
			p.px += dx + p.ax*dt2;
			p.py += dy + p.ay*dt2;
			p.ax = 0.0f;
			p.ay = 0.0f;
		}
		vbo_inst.update_data(stars_);
	}
	
	void render() {
		// update projection matrix
		spo.get_uniform<Eigen::Matrix4f>("proj").set(
			pastry::math_orthogonal_projection(2.0f*WORLD_SIZE, -1.0f, +1.0f));
		spo.get_uniform<Eigen::Matrix4f>("view").set(
			Eigen::Matrix4f::Identity());
		// additive blending
		auto state = pastry::capability(GL_BLEND,true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		// render instances
		spo.use();
		pastry::texture_2d::activate_unit(0);
		tex.bind();
		va.bind();
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, stars_.size());
	}
};

int main()
{
	pastry::initialize();

	pastry::scene_add(std::make_shared<galaxy>());

	pastry::run();

	return 0;
}
