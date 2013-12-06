
#include <pastry/pastry.hpp>
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

constexpr float RAD_MIN = 4.0f;
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

constexpr unsigned GRID_DIM = 256;
constexpr float GRID_W = 5.0f;
constexpr float GRID_MID = 0.5f * GRID_W * GRID_DIM;

struct grid;

struct star
{
	float px, py;
	float vx, vy;
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
		std::uniform_real_distribution<float> r_vel(-1.0f, +1.0f);
		px = r_pos(s_rnd);
		py = r_pos(s_rnd);
		vx = VEL_INIT_SCL*r_vel(s_rnd);
		vy = VEL_INIT_SCL*r_vel(s_rnd);
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

	void update(float t, float dt, std::size_t me, std::vector<star>& other, grid* g);
};

struct grid
{
	std::vector<std::size_t> grid[GRID_DIM*GRID_DIM];

	static std::size_t p2i(float x) {
		int i = static_cast<int>((GRID_MID + x) / GRID_W);
		if(i < 0) return 0;
		if(i >= GRID_DIM) return GRID_DIM-1;
		return i;
	}

	static std::size_t p2i(float x, float y) {
		return p2i(x) + GRID_DIM*p2i(y);
	}

	void build_grid(const std::vector<star>& vp) {
		for(std::size_t k=0; k<vp.size(); k++) {
			std::size_t i = p2i(vp[k].px, vp[k].py);
			grid[i].push_back(k);
		}
	}

	template<typename F>
	void traverse(float px, float py, float r, F f) {
		std::size_t ix = p2i(px);
		std::size_t iy = p2i(py);
		std::size_t ir = std::ceil(r / GRID_W);
		std::size_t x1 = (ix < ir ? 0 : ix - ir);
		std::size_t x2 = std::min<std::size_t>(GRID_DIM-1, ix + ir);
		std::size_t y1 = (iy < ir ? 0 : iy - ir);
		std::size_t y2 = std::min<std::size_t>(GRID_DIM-1, iy + ir);
		for(std::size_t y=y1; y<=y2; y++) {
			for(std::size_t x=x1; x<=x2; x++) {
				for(std::size_t idx : grid[x+GRID_DIM*y]) {
					f(idx);
				}
			}
		}
	}
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

void star::update(float t, float dt, std::size_t me, std::vector<star>& other, grid* g)
{
	const float R_SEARCH = std::sqrt(G * m / A_MIN);
	if(is_active) {
		g->traverse(px, py, R_SEARCH,
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
	pastry::texture tex;

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
			{"vel", GL_FLOAT, 2},
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
				outColor = texture(tex, vuv);
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
		for(int i=0; i<NUM_SUNS; i++) {
			star s{1};
			s.px = SUNS[i][0];
			s.py = SUNS[i][1];
			s.set_radius(RAD_SUN);
			stars_.push_back(s);
		}
		// normal
		std::uniform_real_distribution<float> r_vel(-1.0f, +1.0f);
		for(int i=NUM_SUNS; i<NUM_STARS; i++) {
			star s{1};
			// float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM_STARS);
			// s.px = POS_INIT_RAD*std::cos(phi) + POS_INIT_THICK*r_vel(s_rnd);
			// s.py = POS_INIT_RAD*std::sin(phi) + POS_INIT_THICK*r_vel(s_rnd);
			stars_.push_back(s);
		}
		// passive
		for(int i=0; i<NUM_STARS_PASSIVE; i++) {
			star s{0};
			stars_.push_back(s);
		}
	}
	
	void update(float t, float dt) {
		grid* g = new grid();
		g->build_grid(stars_);
		for(std::size_t k=0; k<stars_.size(); k++) {
			stars_[k].update(t, dt, k, stars_, g);
		}
		dt = 1.0f / 60.0f;
		for(star& p : stars_) {
			p.vx += dt*p.ax;
			p.vy += dt*p.ay;
			p.px += dt*p.vx;
			p.py += dt*p.vy;
			p.ax = 0.0f;
			p.ay = 0.0f;
		}
		vbo_inst.update_data(stars_);
		delete g;
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
		pastry::texture::activate_unit(0);
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
