#include "Rasterizer.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <exception>
#include <limits>
#include <vector>

cg::Rasterizer::Rasterizer(const Camera camera, const std::vector<Scene>& scenes, const rasterization_mode mode, const unsigned int width, const unsigned int height)
	: camera(camera), scenes(scenes), mode(mode), image(width, height), zBuffer(width, height),
	lastRotation(std::chrono::milliseconds::zero()), rotationSpeed(10.0f), rotationAxis(0.0f, 0.0f, 1.0f)
{
	if (this->scenes.size() == 0)
	{
		throw std::runtime_error("There must be at least one scene!");
	}

	this->camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
}

void cg::Rasterizer::draw(const bool rotate)
{
	// Reset image and z-buffer
	this->image.initialize(0.0f);
	this->zBuffer.resize(this->image.get_width(), this->image.get_height());
	this->zBuffer.initialize(std::numeric_limits<float>::max());

	// Rotate scene in front of the camera
	auto alpha = 0.0f;
	vec3 rotationAxis = oneVec3();

	if (rotate)
	{
		std::chrono::milliseconds current = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		
		if (this->lastRotation == std::chrono::milliseconds::zero())
		{
			this->lastRotation = current;
		}
		else
		{
			alpha = this->rotationSpeed * static_cast<float>((current - this->lastRotation).count()) / 1000.0f;
		}

		rotationAxis = this->rotationAxis;
	}
	else
	{
		this->lastRotation = std::chrono::milliseconds::zero();
	}

	const auto transformation = glm::gtc::matrix_transform::rotate(unitMat4(), alpha, rotationAxis);

	// Information message
	switch (this->mode)
	{
	case POINTS:
		std::cout << "Drawing points..." << std::endl;

		break;
	case WIREFRAME:
		std::cout << "Drawing wireframe..." << std::endl;

		break;
	case FILLED:
		std::cout << "Drawing triangles..." << std::endl;
	}

	// Draw each object
	for (const auto obj : this->scenes[this->activeScene].getObjects())
	{
		if (obj->isVisible())
		{
			drawObject(obj, transformation);
		}
	}
}

cg::Camera& cg::Rasterizer::accessCamera()
{
	return this->camera;
}

cg::Scene& cg::Rasterizer::accessScene()
{
	return this->scenes[this->activeScene];
}

const std::vector<cg::Scene>& cg::Rasterizer::getScenes() const
{
	return this->scenes;
}

int& cg::Rasterizer::accessActiveScene()
{
	return this->activeScene;
}

int cg::Rasterizer::getNumScenes() const
{
	return static_cast<int>(this->scenes.size());
}

cg::Rasterizer::rasterization_mode& cg::Rasterizer::accessMode()
{
	return this->mode;
}

cg::image<cg::color_space_t::RGBA>& cg::Rasterizer::accessImage()
{
	return this->image;
}

float& cg::Rasterizer::accessRotationSpeed()
{
	return this->rotationSpeed;
}

cg::vec3& cg::Rasterizer::accessRotationAxis()
{
	return this->rotationAxis;
}

void cg::Rasterizer::drawObject(const std::shared_ptr<cg::SceneObject> object, const mat4& transformation)
{
	// Ger camera transformations
	const auto viewProjection = this->camera.getViewProjection();

	// Draw the mesh
	const auto mesh = object->calculateMesh();
	const auto model = object->getTransformation();

	// Get lights
	auto lights = this->scenes[this->activeScene].getLights();

	// For each triangle
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < mesh.size(); ++i)
	{
		auto triangle = mesh[i];

		// For each triangle corner point
		for (auto& point : triangle.points)
		{
			const auto global_trafo = transformation * model;

			vec4 point_world = zeroVec4();
			vec3 normal_world = zeroVec3();
			// Transform point's position and normal to world space.
			vec4 temp = vec4(point.position, 1.0f);
			point_world = global_trafo * temp;
			temp = vec4(point.normal, 0.0f);
			normal_world = vec3(temp.x, temp.y, temp.z);

			auto color = black();

			for (const auto& light : lights)
			{
				if (light->isVisible())
				{
					const auto colorInfo = light->getColor(Point3D(point_world));

					if (colorInfo.ambient)
					{
						// Add ambient part
						color += point.color * colorInfo.color * colorInfo.intensity;
					}
					else
					{
						// Calculate the illumination of the object according to Lambert's law,
						// and add it to the color already calculated from previous light sources.
						// All relevant information from the light source is stored in colorInfo.
						// Hint: The direction of the ray is from the light source to the object.
						float dot = (-colorInfo.ray.x*point.normal.x) + (-colorInfo.ray.y*point.normal.y) + (-colorInfo.ray.z*point.normal.z);
						float lenSq1 = colorInfo.ray.x*colorInfo.ray.x + colorInfo.ray.y*colorInfo.ray.y + colorInfo.ray.z*colorInfo.ray.z;
						float lenSq2 = point.normal.x*point.normal.x + point.normal.y*point.normal.y + point.normal.z*point.normal.z;
						float cos_angle = dot / sqrt(lenSq1) * sqrt(lenSq2);
						color += point.color * colorInfo.color * colorInfo.intensity * cos_angle * (1.0f/((0.001f + sqrt(lenSq1))*sqrt(lenSq1)));
					}
				}
			}

			point.color = color;

			// Transform to clip space
			auto point_clip = viewProjection * point_world;

			point.validXY = point_clip.x > -point_clip.w && point_clip.x < point_clip.w && point_clip.y > -point_clip.w && point_clip.y < point_clip.w;
			point.validZ = point_clip.z > -point_clip.w && point_clip.z < point_clip.w;

			// Transform to screen space
			point_clip.x = point_clip.x / point_clip.w;
			point_clip.y = point_clip.y / point_clip.w;
			point_clip.z = point_clip.z / point_clip.w;

			point.position.x = (point_clip.x * 0.5f + 0.5f) * this->image.get_width();
			point.position.y = (point_clip.y * -0.5f + 0.5f) * this->image.get_height();
			point.position.z = point_clip.z;
		}

		// Draw triangle
		drawTriangle(triangle);
	}
}

void cg::Rasterizer::drawTriangle(const Triangle& triangle)
{
	switch (this->mode)
	{
	case POINTS:
		rasterizePoints(triangle);

		break;
	case WIREFRAME:
		drawWireframe(triangle);

		break;
	case FILLED:
		rasterizeFilled(triangle);
	}
}

void cg::Rasterizer::rasterizePoints(const Triangle& triangle)
{
	// Draw pixels if they are in front of the camera
	if (triangle.points[0].validXY && triangle.points[0].validZ) setPixel(triangle.points[0].position, triangle.points[0].color);
	if (triangle.points[1].validXY && triangle.points[1].validZ) setPixel(triangle.points[1].position, triangle.points[1].color);
	if (triangle.points[2].validXY && triangle.points[2].validZ) setPixel(triangle.points[2].position, triangle.points[2].color);
}

void cg::Rasterizer::drawWireframe(const Triangle& triangle)
{
	// Create lines
	const std::array<std::pair<Triangle::Point, Triangle::Point>, 3> lines = {
		std::make_pair(triangle.points[0], triangle.points[1]),
		std::make_pair(triangle.points[0], triangle.points[2]), 
		std::make_pair(triangle.points[1], triangle.points[2]) };

	for (const auto& line : lines)
	{
		if ((line.first.validXY && line.first.validZ) || (line.second.validXY && line.second.validZ))
		{
			// Rasterize the line
			rasterizeLine(line.first, line.second);
		}
	}
}

void cg::Rasterizer::rasterizeFilled(const Triangle& triangle)
{
	if (triangle.points[0].validZ && triangle.points[1].validZ && triangle.points[2].validZ)
	{
		// Line-wise fill triangle
		const auto x_min = static_cast<unsigned int>(std::max(static_cast<int>(std::round(std::min(
		{ triangle.points[0].position.x, triangle.points[1].position.x, triangle.points[2].position.x }))), 0));
		const auto x_max = static_cast<unsigned int>(std::max(std::min(static_cast<int>(std::round(std::max(
		{ triangle.points[0].position.x, triangle.points[1].position.x, triangle.points[2].position.x }))), static_cast<int>(this->image.get_width() - 1)), 0));

		const auto y_min = static_cast<unsigned int>(std::max(static_cast<int>(std::round(std::min(
		{ triangle.points[0].position.y, triangle.points[1].position.y, triangle.points[2].position.y }))), 0));
		const auto y_max = static_cast<unsigned int>(std::max(std::min(static_cast<int>(std::round(std::max(
		{ triangle.points[0].position.y, triangle.points[1].position.y, triangle.points[2].position.y }))), static_cast<int>(this->image.get_height() - 1)), 0));

		for (auto y = y_min; y <= y_max; ++y)
		{
			for (auto x = x_min; x <= x_max; ++x)
			{
				// Check whether the x, y coordinates are inside of the triangle
				const Triangle2D triangle_2d(triangle);
				const vec2 coords(x, y);

				if (pointInTriangle(triangle_2d, coords))
				{
					// Calculate interpolated z-value and color
					const auto weights = calculateBarycentricCoords(triangle_2d, coords);

					const auto z = weights[0] * triangle.points[0].position.z + weights[1] * triangle.points[1].position.z + weights[2] * triangle.points[2].position.z;
					const auto color = weights[0] * triangle.points[0].color + weights[1] * triangle.points[1].color + weights[2] * triangle.points[2].color;

					setPixel(Point3D(static_cast<float>(x), static_cast<float>(y), z), color);
				}
			}
		}
	}
}

void cg::Rasterizer::rasterizeLine(const cg::Triangle::Point& point_start, const cg::Triangle::Point& point_end)
{
	///////
	// TODO
	// Implement Bresenham's line algorithm for rasterizing a single line.
	int x0 = static_cast<int>(std::round(point_start.position.x));
	int y0 = static_cast<int>(std::round(point_start.position.y));
	int z0 = static_cast<int>(std::round(point_start.position.z));

	int x1 = static_cast<int>(std::round(point_end.position.x));
	int y1 = static_cast<int>(std::round(point_end.position.y));
	int z1 = static_cast<int>(std::round(point_end.position.z));

	int x0_start = x0;
	int y0_start = y0;
	int z0_start = z0;

	int x1_end = x1;
	int y2_end = y1;
	int z1_end = z1;

	int direction_x, direction_y, direction_z;

	int distance_x = abs(x1-x0);
	if (x0<x1) {
		direction_x = 1;
	} else {
		direction_x = -1;
	}

	int distance_y = abs(y1-y0);
	if (y0<y1) {
		direction_y = 1;
	} else {
		direction_y = -1;
	}

	int distance_z = abs(z1-z0);
	if (z0<z1) {
		direction_z = 1;
	} else {
		direction_z = -1;
	}

	int distance_max = std::max(distance_x, std::max(distance_y, distance_z));
	x1 = y1 = z1 = distance_max / 2;

	Color c = white();

	c = point_start.color;
	float interpolation;
	setPixel(Point3D(x0,y0,z0), c);

	for(int i = distance_max; i > 0; i--) {
		x1 -= distance_x;
		if (x1 < 0)
		{
			x1 += distance_max;
			x0 += direction_x;
		}

		y1 -= distance_y;
		if (y1 < 0)
		{
			y1 += distance_max;
			y0 += direction_y;
		}

		z1 -= distance_z;
		if (z1 < 0) {
			z1 += distance_max;
			z0 += direction_z;
		}

		interpolation = sqrt(pow(x0_start, 2) + pow(y0_start, 2) + pow(z0_start, 2)) /
		sqrt(pow(x1_end - x0_start, 2)+ pow(x1_end - y0_start, 2) + pow(z1_end - z0_start, 2));
		c = (1-interpolation) * point_start.color + interpolation * point_end.color;

		setPixel(Point3D(x0,y0,z0), c);
	}
}

void cg::Rasterizer::setPixel(const Point3D& point, Color color)
{
	const auto x = static_cast<int>(std::round(point.x));
	const auto y = static_cast<int>(std::round(point.y));
	const auto z = point.z;

	#pragma omp critical(image_access)
	{
		// Only draw if:
		//  - (x, y) coordinates are within the image
		//  - z is within the camera's range [near, far]
		//  - z is smaller than or equal to the one stored in the z-buffer
		if (x >= 0 && x < static_cast<int>(this->image.get_width()) && y >= 0 && y < static_cast<int>(this->image.get_height())
			&& z > this->camera.getNear() && z < this->camera.getFar() && this->zBuffer.at(x, y)[0] >= z)
		{
			// If z-buffer equals z-value, only draw pixel if it is lighter than the current one
			if (this->zBuffer.at(x, y)[0] == z)
			{
				const float r_weight = 0.2989f;
				const float g_weight = 0.5870f;
				const float b_weight = 0.1140f;

				const auto luminance_old = r_weight * this->image.at(x, y)[0] + g_weight * this->image.at(x, y)[1] + b_weight * this->image.at(x, y)[2];
				const auto luminance_new = r_weight * color.r + g_weight * color.g + b_weight * color.b;

				if (luminance_new < luminance_old)
				{
					color = vec4(this->image.at(x, y)[0], this->image.at(x, y)[1], this->image.at(x, y)[2], this->image.at(x, y)[3]);
				}
			}

			this->image.at(x, y) = { color.r, color.g, color.b, color.a };
			this->zBuffer.at(x, y)[0] = z;
		}
	}
}