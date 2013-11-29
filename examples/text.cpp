
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::add_renderling([]() {
		pastry::render_text(80, 100, "Hello world :)");
		pastry::render_text(80, 250, "CRAZY text by STB!");
	});

	pastry::run();

	return 0;
}
