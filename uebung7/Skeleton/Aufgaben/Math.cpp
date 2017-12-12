#include "Math.h"

#include <cmath>

float cg::pi()
{
	return static_cast<float>(std::atan(1)) * 4.0f;
}

bool cg::equals(float a, float b, float eps)
{
	return a + eps >= b && a - eps <= b;
}

cg::vec2 cg::zeroVec2()
{
	cg::vec2 vector;
	vector[0] = vector[1] = 0.0f;

	return vector;
}

cg::vec3 cg::zeroVec3()
{
	cg::vec3 vector;
	vector[0] = vector[1] = vector[2] = 0.0f;

	return vector;
}

cg::vec4 cg::zeroVec4()
{
	cg::vec4 vector;
	vector[0] = vector[1] = vector[2] = vector[3] = 0.0f;

	return vector;
}

cg::vec2 cg::oneVec2()
{
	cg::vec2 vector;
	vector[0] = vector[1] = 1.0f;

	return vector;
}

cg::vec3 cg::oneVec3()
{
	cg::vec3 vector;
	vector[0] = vector[1] = vector[2] = 1.0f;

	return vector;
}

cg::vec4 cg::oneVec4()
{
	cg::vec4 vector;
	vector[0] = vector[1] = vector[2] = vector[3] = 1.0f;

	return vector;
}

cg::mat2 cg::zeroMat2()
{
	cg::mat2 matrix;
	matrix[0][0] = matrix[0][1] = 0.0f;
	matrix[1][0] = matrix[1][1] = 0.0f;

	return matrix;
}

cg::mat3 cg::zeroMat3()
{
	cg::mat3 matrix;
	matrix[0][0] = matrix[0][1] = matrix[0][2] = 0.0f;
	matrix[1][0] = matrix[1][1] = matrix[1][2] = 0.0f;
	matrix[2][0] = matrix[2][1] = matrix[2][2] = 0.0f;

	return matrix;
}

cg::mat4 cg::zeroMat4()
{
	cg::mat4 matrix;
	matrix[0][0] = matrix[0][1] = matrix[0][2] = matrix[0][3] = 0.0f;
	matrix[1][0] = matrix[1][1] = matrix[1][2] = matrix[1][3] = 0.0f;
	matrix[2][0] = matrix[2][1] = matrix[2][2] = matrix[2][3] = 0.0f;
	matrix[3][0] = matrix[3][1] = matrix[3][2] = matrix[3][3] = 0.0f;

	return matrix;
}

cg::mat2 cg::unitMat2()
{
	cg::mat2 matrix = zeroMat2();
	matrix[0][0] = matrix[1][1] = 1.0f;

	return matrix;
}

cg::mat3 cg::unitMat3()
{
	cg::mat3 matrix = zeroMat3();
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0f;

	return matrix;
}

cg::mat4 cg::unitMat4()
{
	cg::mat4 matrix = zeroMat4();
	matrix[0][0] = matrix[1][1] = matrix[2][2] = matrix[3][3] = 1.0f;

	return matrix;
}

cg::Color cg::black()
{
	cg::Color vector;
	vector[0] = vector[1] = vector[2] = 0.0f;
	vector[3] = 1.0f;

	return vector;
}

cg::Color cg::white()
{
	cg::Color vector;
	vector[0] = vector[1] = vector[2] = vector[3] = 1.0f;

	return vector;
}

cg::Color cg::red()
{
	cg::Color vector;
	vector[0] = vector[3] = 1.0f;
	vector[1] = vector[2] = 0.0f;

	return vector;
}

cg::Color cg::green()
{
	cg::Color vector;
	vector[1] = vector[3] = 1.0f;
	vector[0] = vector[2] = 0.0f;

	return vector;
}

cg::Color cg::blue()
{
	cg::Color vector;
	vector[2] = vector[3] = 1.0f;
	vector[0] = vector[1] = 0.0f;

	return vector;
}

cg::Triangle2D::Triangle2D()
{ }

cg::Triangle2D::Triangle2D(const cg::Triangle& other)
{
	this->points = {
		Point{ Point2D(other.points[0].position), other.points[0].color },
		Point{ Point2D(other.points[1].position), other.points[1].color },
		Point{ Point2D(other.points[2].position), other.points[2].color }
	};
}

namespace
{
	bool sameSide(const cg::Point2D& first, const cg::Point2D& second, const cg::Point2D& line_start, const cg::Point2D& line_end)
	{
		const auto line = line_end - line_start;
		const auto first_line = first - line_start;
		const auto second_line = second - line_start;

		const float cp1 = line.x * first_line.y - line.y * first_line.x;
		const float cp2 = line.x * second_line.y - line.y * second_line.x;

		return glm::core::function::geometric::dot(cp1, cp2) >= 0.0f;
	}
}

bool cg::pointInTriangle(const cg::Triangle2D& triangle, const cg::Point2D& position)
{
	return sameSide(position, triangle.points[0].position, triangle.points[1].position, triangle.points[2].position)
		&& sameSide(position, triangle.points[1].position, triangle.points[0].position, triangle.points[2].position)
		&& sameSide(position, triangle.points[2].position, triangle.points[0].position, triangle.points[1].position);
}

cg::vec3 cg::calculateBarycentricCoords(const Triangle2D& triangle, const cg::Point2D& position)
{
	// Calculate the barycentric coordinates of a triangle.
	// Hint: You can safely assume, that a given position is always inside the triangle.
	float Area_a = get_Area(triangle.points[1].position, triangle.points[2].position, position);
	float Area_b = get_Area(triangle.points[2].position, triangle.points[0].position, position);
	float Area_c = get_Area(triangle.points[0].position, triangle.points[1].position, position);

	const float d = Area_a + Area_b + Area_c;

	Area_a = Area_a / d;
	Area_b = Area_b / d;
	Area_c = Area_c / d;

	float r, g, b;
	r = Area_a * triangle.points[0].color.r + Area_b * triangle.points[1].color.r + Area_c * triangle.points[2].color.r;
	g = Area_a * triangle.points[0].color.g + Area_b * triangle.points[1].color.g + Area_c * triangle.points[2].color.g;
	b = Area_a * triangle.points[0].color.b + Area_b * triangle.points[1].color.b + Area_c * triangle.points[2].color.b;

	return vec3(r, g, b);
}

float cg::get_Area(cg::Point2D A, cg::Point2D B, cg::Point2D C) {
	float s, a, b, c;
	a = sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
	b = sqrt((C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y));
	c = sqrt((A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y));
	s = (a + b + c) / 2.0f;
	return sqrt(s * (s-a) * (s - b) * (s - c));
}

cg::Point3D cg::sphericalToCartesian(const float r, const float teta, const float phi)
{
	// Convert the spherical coordinates to cartesian coordinates.
	// Use the ISO convention, where teta is in [0, pi) and phi is in [0, 2*pi).
	return Point3D(r*sin(teta)*cos(phi), r*sin(teta)*sin(phi), r*cos(teta));
}