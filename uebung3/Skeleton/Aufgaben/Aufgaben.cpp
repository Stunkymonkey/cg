#include "Aufgaben.h"

#include "Image.h"
#include "ImageIO.h"
#include "ImageConverter.h"

#include <exception>
#include <iostream>
#include <string>

int main(const int argc, const char** argv)
{
	// Read command line arguments
	if (argc != 3)
	{
		std::cerr << "Error: No input and output file specified" << std::endl;
		std::cout << "Call program with parameters <source> <target>" << std::endl << std::endl;
	}

	std::string source_file(argv[1]);
	std::string target_file(argv[2]);

	std::cout << "Source file: " << source_file << std::endl;
	std::cout << "Target file: " << target_file << std::endl << std::endl;

	// Ask user to execute one of the exercises
	unsigned int exercise;
	std::cout << "Select exercise [1-3]: ";
	std::cin >> exercise;

	switch (exercise)
	{
	case 1:
		aufgabe1(source_file, target_file);
		break;
	case 2:
		aufgabe2(source_file, target_file);
		break;
	case 3:
		aufgabe3(source_file, target_file);
		break;
	default:
		std::cerr << "Invalid exercise: " << exercise << std::endl;
	}

	return 0;
}

void aufgabe1(const std::string& source_file, const std::string& target_file)
{
	try
	{
		///////
		// TODO
		// Load an RGB image, convert it to grayscale and save the grayscale image.
		// Use the classes provided in ImageIO and ImageConverter.

		std::cout << "File successfully created" << std::endl << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}
}

void aufgabe2(const std::string& source_file, const std::string& target_file)
{
	try
	{
		///////
		// TODO
		// Load a grayscale image, convert it to black-and-white and save the image.
		// Use the classes provided in ImageIO and ImageConverter.

		std::cout << "File successfully created" << std::endl << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}
}

void aufgabe3(const std::string& source_file, const std::string& target_file)
{
	try
	{
		///////
		// TODO
		// Load an RGB image, convert it to HSV and back to RGB. Save the resulting RGB image.
		// Use the classes provided in ImageIO and ImageConverter.

		std::cout << "File successfully created" << std::endl << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}
}