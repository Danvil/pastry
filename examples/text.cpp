
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::scene_add([]() {
		pastry::text_render(80, 100, "Hello world :)");
		pastry::text_render(80, 250, "CRAZY text by STB!");
	});

	pastry::run();

	return 0;
}
