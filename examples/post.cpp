
#include <pastry/pastry.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

class kitten
{
public:
	static constexpr float SIZE = 0.07f;
	std::shared_ptr<pastry::sprite> s;
	float gx, gy;
	kitten(float x, float y, const std::shared_ptr<pastry::sprite>& ns) {
		s = ns;
		s->x = x;
		s->y = y;
		s->sx = SIZE;
		s->sy = SIZE;
		gx = x;
		gy = y;
	}
	void update(float t, float dt) {
		std::uniform_real_distribution<float> uniform_dist(0, 512.0f);
 		float dx = s->x - gx;
		float dy = s->y - gy;
		float dist = std::sqrt(dx*dx + dy*dy);
		if(dist < 0.01f) {
			gx = uniform_dist(s_rnd);
			gy = uniform_dist(s_rnd);
		}
		s->x += (gx - s->x)*dt*0.34f;
		s->y += (gy - s->y)*dt*0.34f;
	}
};

class kitten_manager : public pastry::renderling
{
	pastry::sprite_group sprites_;
	std::vector<kitten> kittens_;
public:
	static constexpr int NUM = 100;
	kitten_manager() {
		pastry::sprites_add_sheet({
			"kitten",
			pastry::texture_load("assets/kitten.jpg"),
			{
				{"kitten", 0, 0, 1024, 786}
			}
		}, false);
		// create data
		for(int i=0; i<NUM; i++) {
			std::shared_ptr<pastry::sprite> s = sprites_.add_sprite("kitten");
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
			kitten k{256.0f+100.0f*std::cos(phi), 256.0f+100.0f*std::sin(phi), s};
			kittens_.push_back(k);
		}
	}
	void update(float t, float dt) {
		for(kitten& k : kittens_) {
			k.update(t,dt);
		}
		sprites_.update(t,dt);
	}
	void render() {
		sprites_.render();
	}
};

int main()
{
	pastry::initialize();

	pastry::scene_add(std::make_shared<kitten_manager>());

	pastry::postfx_add(
		"uniform float offset;"
		"vec4 sfx(vec2 uv) {"
		"	uv.x += sin((4*uv.y + offset)*2*3.14159)/80;"
		"	return sfx_read_fb(uv);"
		"}",
		[](float t, float dt, const pastry::program& spo) {
			spo.get_uniform<float>("offset").set(t * .75);
		}
	);

	pastry::postfx_add(
		"uniform float a;"
		"vec4 sfx(vec2 uv) {"
		"	return vec4(1,a,a,1) * sfx_read_fb(uv);"
		"}",
		[](float t, float dt, const pastry::program& spo) {
			float q = std::fmod(t, 2.0f) - 1.0f;
			spo.get_uniform<float>("a").set(1.0f - q*q);
		}
	);

	pastry::postfx_add(
		"vec4 sfx(vec2 uv) {"
		"	return   sfx_read_fb(uv+vec2(+1,0)/postfx_dim)"
		"	       + sfx_read_fb(uv+vec2(-1,0)/postfx_dim)"
		"	       + sfx_read_fb(uv+vec2(0,+1)/postfx_dim)"
		"	       + sfx_read_fb(uv+vec2(0,-1)/postfx_dim)"
		"	       - 4*sfx_read_fb(uv);"
		"}"
	);

	pastry::run();

	return 0;
}
