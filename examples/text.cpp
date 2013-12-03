
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::scene_add([]() {
		pastry::text_render(80, 400, "Hello world :)");
		pastry::text_render(80, 250, "CRAZY text by STB!");
		pastry::text_render(0, 0, "0/0");
	});

	pastry::run();

	return 0;
}
