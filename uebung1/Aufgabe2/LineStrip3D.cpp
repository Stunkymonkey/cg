// CG Assignment 1 Exercise 2 WS 17/18

#include "LineStrip3D.hpp"
#include <stdlib.h>
#include <cmath>
#include <list>

bool LineStrip3D::operator<(LineStrip3D const& rhs)
{
	// Implement comparison based on length.
	return computeLength() < rhs.computeLength();
}

void LineStrip3D::addPoint(Point3D p)
{
	// Implement adding a given point to the line strip.
	points.push_back(p);
}

void LineStrip3D::removePoint(size_t idx)
{
	// Implement the removal of the point with the given index.
	std::list<Point3D>::iterator j = points.begin();
	for(int i = 0; i <= idx; i++){
		++j;
	}
	points.erase(j);
}

size_t	LineStrip3D::getPointCount() const
{
	// Return the number of points of the line strip.
	return points.size();
}

float LineStrip3D::computeLength() const
{
	// that is, the sum of the lengths of all segements.
	float result;
	std::list<Point3D>::const_iterator i = points.begin();
	std::list<Point3D>::const_iterator j = points.begin();
	++j;
	for(; j != points.end(); ++i, ++j){
		Point3D first = *i;
		Point3D second = *j;
		result = result + std::sqrt(std::pow(second .x() - first.x(), 2) +
									std::pow(second .y() - first.y(), 2) +
									std::pow(second .z() - first.z(), 2));
	}
	return result;
}