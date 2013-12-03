
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	pastry::initialize();

	pastry::scene_add(
		[]() {
			pastry::text_render(100,400,"Press W!");
			if(pastry::key_is_pressed('W')) {
				pastry::text_render(100,200,"You press W!");
			}
		}
	);

	pastry::run();

	return 0;
}
