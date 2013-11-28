
#include <pastry/pastry.hpp>
#include <pastry/sprites.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

class thingy_manager
: public pastry::renderling
{
	pastry::sprites::sprite_group sprites_;
public:
	static constexpr int NUM = 12;
	thingy_manager() {
		pastry::sprites::add_sprite_sheet({
			"player",
			"assets/player.png",
			{
				{"a00",  0*17, 0, 17, 17},
				{"a01",  1*17, 0, 17, 17},
				{"a02",  2*17, 0, 17, 17},
				{"a03",  3*17, 0, 17, 17},
				{"a04",  4*17, 0, 17, 17},
				{"a05",  5*17, 0, 17, 17},
				{"a06",  6*17, 0, 17, 17},
				{"a07",  7*17, 0, 17, 17},
				{"a08",  8*17, 0, 17, 17},
				{"a09",  9*17, 0, 17, 17},
				{"a10", 10*17, 0, 17, 17},
				{"a11", 11*17, 0, 17, 17}
			}
		});
		pastry::sprites::add_sprite_animation({"blah", {
			{"a00", 0.13f},
			{"a01", 0.13f},
			{"a02", 0.13f},
			{"a03", 0.13f},
			{"a04", 0.13f},
			{"a05", 0.13f},
			{"a06", 0.13f},
			{"a07", 0.13f},
			{"a08", 0.13f},
			{"a09", 0.13f},
			{"a10", 0.13f},
			{"a11", 0.13f}
		}});
		// create data
		for(int i=0; i<NUM; i++) {
			std::shared_ptr<pastry::sprites::sprite> s = sprites_.add_sprite("blah");
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
			s->x = 0.7f*std::cos(phi);
			s->y = 0.7f*std::sin(phi);
			s->sx = 0.1f;
			s->sy = 0.1f;
			s->t = static_cast<float>(i)*0.13f;
		}
	}
	void update(float t, float dt) {
		sprites_.update(t, dt);
	}
	void render() {
		sprites_.render();
	}
};

int main()
{
	pastry::initialize();

	pastry::add_renderling(std::make_shared<thingy_manager>());

	pastry::run();

	return 0;
}
