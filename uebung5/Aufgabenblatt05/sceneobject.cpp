#include "sceneobject.h"

static const float PI = 3.1415926f;

/**
 * @brief Plane::intersect
 */
bool Plane::intersect(const Ray &ray, float &t) const
{
    float denom = this->_normal.dot(ray.dir);
    if (denom > 1e-6)   // avoid zero div
    {
        Vec3f origin2point = this->_point - ray.origin;
        t = origin2point.dot(this->_normal) / denom;
        return (t >= 0);
    }
    return false;
}

/**
 * @brief Plane::getSurfaceColor
 */
Vec3f Plane::getSurfaceColor(const Vec3f &p_hit) const
{
    // generate grey chess board pattern
    const float freq = 0.125f;
    float s = cos(p_hit.x * 2.f*PI * freq) * cos(p_hit.z * 2.f*PI * freq);
    return Vec3f(0.2f) + (s > 0)*Vec3f(0.4f);
}


/**
 * @brief Sphere::intersect
 */
bool Sphere::intersect(const Ray &ray, float &t) const
{
    ///////////
    // TODO 2.2
    // Implement a ray-sphere intersection test.
    // cf. lecture slides 46ff
	//
    // END TODO
    ///////////

	float a = ray.dir.dot(ray.dir);
	float b = (2 * ray.dir).dot(ray.origin - this->_center);
	float c = (ray.origin - this->_center).dot((ray.origin - this->_center)) - (this->_radius * this->_radius);

	float D = pow(b, 2) - 4 * a * c;

	if (D > 0)
	{
		float t1 = (-b + sqrt(D)) / (2 * a);
		float t2 = (-b - sqrt(D)) / (2 * a);
		if (t1 < t2) {
			t = t1;
		}
		else {
			t = t2;
		}
		return true;
	}
	else if (D == 0)
	{
		t = -b / (2*a);
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Sphere::getSurfaceColor
 */
Vec3f Sphere::getSurfaceColor(const Vec3f &p_hit) const
{
    return this->_color;
}
