
#include <pastry/pastry.hpp>
#include <pastry/sprites.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

int main()
{
	pastry::initialize();

	std::cout << "Initialize sprites" << std::endl;

	pastry::sprites_initialize();

	pastry::sprites_add_sheet({
		"player",
		pastry::texture_load("assets/player.png"),
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

	pastry::sprites_add_animation({"blah", {
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

	std::cout << "Create sprites" << std::endl;

	std::shared_ptr<pastry::sprite_group> sprites = std::make_shared<pastry::sprite_group>();

	constexpr int NUM = 12;
	for(int i=0; i<NUM; i++) {
		pastry::sprite_ptr s = sprites->add_sprite("blah");
		float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
		s->x = 256.0f+100.0f*std::cos(phi);
		s->y = 256.0f+100.0f*std::sin(phi);
		s->sx = 1.f;
		s->sy = 1.f;
		s->t = static_cast<float>(i)*0.13f;
	}

	pastry::scene_add(sprites);

	pastry::run();

	return 0;
}
