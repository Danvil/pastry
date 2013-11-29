
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::add_renderling(
		[]() {
			pastry::render_text(100,100,"Press W!");
			if(pastry::is_key_pressed('W')) {
				pastry::render_text(100,200,"You press W!");
			}
		}
	);

	pastry::run();

	return 0;
}
