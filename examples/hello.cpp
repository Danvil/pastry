
#include <pastry/pastry.hpp>
#include <iostream>

int main()
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
