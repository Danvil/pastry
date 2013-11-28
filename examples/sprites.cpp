
#include <pastry/pastry.hpp>
#include <random>
#include <map>

struct sprite_template
{
	std::string name;
	float u, v;
	float su, sv;
};

struct sprite
{
	float x, y;
	float sx, sy;
	std::shared_ptr<sprite_template> def;
};

class sprites
{
public:
private:
	struct vertex {
		float x, y;
		float u, v;
	};
	pastry::array_buffer vbo;
	pastry::program spo;
	pastry::vertex_array vao;
	pastry::texture tex;
	std::map<std::string,std::shared_ptr<sprite_template>> sprite_templates_;
	std::vector<std::shared_ptr<sprite>> sprites_;
	std::vector<vertex> vertices_;
public:
	sprites() {
		vbo = pastry::array_buffer({
			{"xy", GL_FLOAT, 2},
			{"uv", GL_FLOAT, 2}
		});
		std::string vert_src = PASTRY_GLSL(
			in vec2 position;
			in vec2 texcoord;
			out vec2 Texcoord;
			void main() {
				gl_Position = vec4(position, 0.0, 1.0f);
				Texcoord = texcoord;
			}
		);
		std::string frag_src = PASTRY_GLSL(
			in vec2 Texcoord;
			uniform sampler2D tex;
			out vec4 outColor;
			void main() {
				outColor = texture(tex, Texcoord);
			}
		);
		spo = pastry::program(vert_src, frag_src);
		vao = pastry::vertex_array(spo, {
			{"position", vbo, "xy"},
			{"texcoord", vbo, "uv"}
		});
		tex = pastry::load_texture("assets/kitten.jpg");
		spo.get_uniform<int>("tex").set(0);
	}
	void set_template(const std::string& name, const sprite_template& st) {
		sprite_templates_[name] = std::make_shared<sprite_template>(st);
	}
	std::shared_ptr<sprite_template> get_template(const std::string& name) {
		return sprite_templates_[name];
	}
	std::shared_ptr<sprite> create_sprite(const std::string& name) {
		auto p = std::make_shared<sprite>();
		p->def = get_template(name);
		sprites_.push_back(p);
		return p;
	}
	void remove_sprite(const std::shared_ptr<sprite>& s) {
		auto it = std::find(sprites_.begin(), sprites_.end(), s);
		if(it != sprites_.end()) {
			sprites_.erase(it);
		}
	}
	void create_vertices() {
		vertices_.clear();
		vertices_.reserve(sprites_.size()*6);
		for(const std::shared_ptr<sprite>& sp : sprites_) {
			const sprite& s = *sp;
			const sprite_template& st = *s.def;
			vertices_.push_back(vertex{s.x-s.sx, s.y+s.sy, st.u      , st.v+st.sv});
			vertices_.push_back(vertex{s.x+s.sx, s.y+s.sy, st.u+st.su, st.v+st.sv});
			vertices_.push_back(vertex{s.x+s.sx, s.y-s.sy, st.u+st.su, st.v      });
			vertices_.push_back(vertex{s.x-s.sx, s.y+s.sy, st.u      , st.v+st.sv});
			vertices_.push_back(vertex{s.x+s.sx, s.y-s.sy, st.u+st.su, st.v      });
			vertices_.push_back(vertex{s.x-s.sx, s.y-s.sy, st.u      , st.v      });
		}
		vbo.data(vertices_, GL_DYNAMIC_DRAW);
	}
	void render() {
		vbo.bind();
		pastry::texture::activate_unit(0);
		tex.bind();
		spo.use();
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
	}
};

class kitten
{
private:
	static std::default_random_engine s_rnd;
public:
	static constexpr float SIZE = 0.07f;
	std::shared_ptr<sprite> s;
	float gx, gy;
	kitten(float x, float y, const std::shared_ptr<sprite>& ns) {
		s = ns;
		s->x = x;
		s->y = y;
		s->sx = SIZE;
		s->sy = SIZE;
		gx = x;
		gy = y;
	}
	void update(float t, float dt) {
		std::uniform_real_distribution<float> uniform_dist(-1.0f, +1.0f);
 		float dx = s->x - gx;
		float dy = s->y - gy;
		float dist = std::sqrt(dx*dx + dy*dy);
		if(dist < 0.01f) {
			gx = uniform_dist(s_rnd);
			gy = uniform_dist(s_rnd);
		}
		s->x += (gx - s->x)*dt*0.04f;
		s->y += (gy - s->y)*dt*0.04f;
	}
};

std::default_random_engine kitten::s_rnd;

class kittens : public pastry::renderling
{
	sprites sprites_;
	std::vector<kitten> kittens_;
public:
	static constexpr int NUM = 64;
	kittens() {
		// create templates
		sprite_template st;
		st.u = 0.0f;
		st.v = 0.0f;
		st.su = 1.0f;
		st.sv = 1.0f;
		sprites_.set_template("kitten", st);
		// create data
		for(int i=0; i<NUM; i++) {
			std::shared_ptr<sprite> s = sprites_.create_sprite("kitten");
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
			kitten k{0.7f*std::cos(phi), 0.7f*std::sin(phi), s};
			kittens_.push_back(k);
		}
	}
	void update(float t, float dt) {
		for(kitten& k : kittens_) {
			k.update(t,dt);
		}
		sprites_.create_vertices();
	}
	void render() {
		sprites_.render();
	}
};

int main()
{
	pastry::initialize();

	pastry::add_renderling(std::make_shared<kittens>());

	pastry::run();

	return 0;
}
