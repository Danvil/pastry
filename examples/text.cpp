
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::text_load_font("dejavu", "assets/DejaVuSans.ttf");

	pastry::scene_add([]() {
		pastry::text_render(80, 400, "Hello world :)");
		pastry::text_render(80, 250, "CRAZY text by STB!");
		pastry::text_render(0, 0, "0/0");
		pastry::text_render(200, 100, "overlap");
		pastry::text_render(210, 110, "overlap");
		pastry::text_render(220, 90, "overlap");
		pastry::text_render(320, 300, "red", {1,0,0,1});
		pastry::text_render(330, 320, "yellow", {1,1,0,1});
		pastry::text_render(340, 340, "green", {0,1,0,1});
		pastry::text_render(350, 360, "cyan", {0,1,1,1});
		pastry::text_render(360, 380, "blue", {0,0,1,1});
		pastry::text_render(370, 400, "magenta", {1,0,1,1});
	});

	pastry::run();

	return 0;
}
