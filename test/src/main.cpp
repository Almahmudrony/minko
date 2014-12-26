#include "gtest/gtest.h"

#include "minko/Minko.hpp"
#include "minko/MinkoTests.hpp"

#include "minko/MinkoSDL.hpp"

using namespace minko;

void wait()
{
	std::cout << "Press ENTER to continue...";
	std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
}

int main(int argc, char **argv)
{
	auto canvas = Canvas::create("Minko Tests", 640, 480);

	::testing::InitGoogleTest(&argc, argv);

    MinkoTests::canvas(canvas);

	auto output = RUN_ALL_TESTS();

	//wait();

	return output;
}
