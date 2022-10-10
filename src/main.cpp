#include <iostream>

#include "App.h"

int main()
{
	App triangle_app;

	try {
		triangle_app.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
