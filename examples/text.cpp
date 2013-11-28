
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::add_renderling([]() {
		pastry::render_text(-0.5f, 0.0f, "Hello world :)");
		pastry::render_text(-0.5f, -0.5f, "CRAZY text by STB!");
	});

	pastry::run();

	return 0;
}
