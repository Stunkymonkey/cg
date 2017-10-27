// CG Assignment 1 Exercise 1 WS 17/18

#include <iostream>
#include <cmath>

// Write a function that computes the circumference of a circle given its radius as input.
float circumference(float radius)
{
	return 2 * M_PI * radius;
}

int main()
{
	float radius;

	std::cout << "Insert circle radius:" << std::endl;
	
	// Get user input from the command line and store it in radius.
	// Check if input is valid.
	if (!(std::cin >> radius)) {
		std::cout << "You did not enter a number!" << std::endl;
		return 1;
	}
	
	// Call the function circumference and output the computed value to the command line.
	std::cout << circumference(radius) << std::endl;

	return 0;
}

