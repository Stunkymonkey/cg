// CG Assignment 1 Exercise 2 WS 17/18

#include "Point3D.hpp"
#include "LineStrip3D.hpp"

#include <iostream>
#include <list>


int main()
{
	// Create a container for linestrips
	std::list<LineStrip3D> linestrips;

	// Add three linestrips to the container and add a couple of points to each linestrip
	LineStrip3D x = LineStrip3D();
	LineStrip3D y = LineStrip3D();
	LineStrip3D z = LineStrip3D();
	Point3D a = Point3D(1.0, 0.0, 0.0);
	Point3D b = Point3D(0.0, 1.0, 1.0);
	Point3D c = Point3D(1.0, 0.0, 1.0);
	Point3D d = Point3D(0.0, 0.0, 0.0);
	Point3D e = Point3D(4.0, 0.0, 0.0);
	x.addPoint(a);
	x.addPoint(b);
	y.addPoint(b);
	y.addPoint(c);
	z.addPoint(d);
	z.addPoint(e);
	linestrips.push_back(x);
	linestrips.push_back(y);
	linestrips.push_back(z);

	std::cout << "==================================" << std::endl;
	std::cout << "Length of all linestrips:" << std::endl;

	// Print the length of all line strips on the command line
	std::list<LineStrip3D>::iterator display;
	for (display = linestrips.begin(); display != linestrips.end(); ++display){
		std::cout << display->computeLength() << "\n";
	}
	
	std::cout << "==================================" << std::endl;
	std::cout << "Length of all linestrips (sorted):" << std::endl;

	///////
	// TODO
	// Sort the line strips in the container based on their length.
	// For validation, print the lengths of all linestrips on the command line one more time.
	
	for(int i = 0; i <= linestrips.size(); i++){
		std::list<LineStrip3D>::iterator min;
		std::list<LineStrip3D>::iterator itter = linestrips.begin();
		for(; itter != linestrips.end(); ++itter){
			if(*itter < *min){
				min = itter;
			}
		}
		std::cout << (*min).computeLength() << "\n";
		linestrips.erase(min);
	}

	return 0;
}