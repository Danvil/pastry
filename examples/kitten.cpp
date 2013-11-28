
#include <pastry/pastry.hpp>
#include <pastry/sprites.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

class kitten
{
private:
public:
	static constexpr float SIZE = 0.07f;
	std::shared_ptr<pastry::sprites::sprite> s;
	float gx, gy;
	kitten(float x, float y, const std::shared_ptr<pastry::sprites::sprite>& ns) {
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

class kitten_manager : public pastry::renderling
{
	pastry::sprites::sprite_group sprites_;
	std::vector<kitten> kittens_;
public:
	static constexpr int NUM = 100;
	kitten_manager() {
		pastry::sprites::add_sprite_sheet({
			"kitten",
			pastry::load_texture("assets/kitten.jpg"),
			{
				{"kitten", 0, 0, 1024, 786}
			}
		});
		// create data
		for(int i=0; i<NUM; i++) {
			std::shared_ptr<pastry::sprites::sprite> s = sprites_.add_sprite("kitten");
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
			kitten k{0.7f*std::cos(phi), 0.7f*std::sin(phi), s};
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

	pastry::add_renderling(std::make_shared<kitten_manager>());

	pastry::run();

	return 0;
}
