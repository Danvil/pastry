
#include <pastry/pastry.hpp>
#include <iostream>

void update(float t, float dt)
{
	if(pastry::is_key_pressed('W')) {
		std::cout << "W is pressed" << std::endl;
	}
	if(pastry::is_key_pressed('A')) {
		std::cout << "A is pressed" << std::endl;
	}
	if(pastry::is_key_pressed('S')) {
		std::cout << "S is pressed" << std::endl;
	}
	if(pastry::is_key_pressed('D')) {
		std::cout << "D is pressed" << std::endl;
	}
}

void render()
{
	pastry::render_text(80, 100, "Hello world :)");
	pastry::render_text(80, 250, "CRAZY text by STB!");
}

int main()
{
	pastry::initialize();

	pastry::add_renderling(render, update);

	pastry::run();

	return 0;
}
