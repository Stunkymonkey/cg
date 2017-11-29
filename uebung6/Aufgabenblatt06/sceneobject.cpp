#include "sceneobject.h"

static const float PI = 3.1415926f;

/**
 * @brief Plane::intersect
 */
bool Plane::intersect(const Ray &ray, float &t) const
{
    float denom = this->_normal.dot(ray.dir);
    if (denom < -1e-6)   // avoid zero div
    {
        //Vec3f origin2point = this->_point - ray.origin;
		Vec3f origin2point = ray.origin - this->_point;
        t = origin2point.dot(this->_normal) / -denom;
        return (t >= 0);
    }
    return false;
}

Vec3f Plane::getSurfaceNormal(const Vec3f &p_hit) const
{
	return this->_normal;
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

PhongCoefficients Plane::getPhongCoefficients(const Vec3f & p_hit) const
{
	// generate grey chess board pattern
	const float freq = 0.125f;
	float s = cos(p_hit.x * 2.f*PI * freq) * cos(p_hit.z * 2.f*PI * freq);
	Vec3f color = Vec3f(0.2f) + (s > 0)*Vec3f(0.4f);

	PhongCoefficients phongCoeff;
	std::get<0>(phongCoeff) = color;
	std::get<1>(phongCoeff) = color;
	std::get<2>(phongCoeff) = Vec3f(1.0f);
	std::get<3>(phongCoeff) = 42.0f;

	return phongCoeff;
}


/**
 * @brief Sphere::intersect
 */
bool Sphere::intersect(const Ray &ray, float &t) const
{
    
    // Implement a ray-sphere intersection test.
    // cf. lecture slides 46ff
    //

    float t0 = -1;
    float t1 = -1;
#if 0
    // geometric solution
    Vec3f L = this->_center - ray.origin;
    float tca = L.dot(ray.dir);
    if (tca < 0)
        return false;
    float d2 = L.dot(L) - tca * tca;
    if (d2 > this->_radius*this->_radius)
        return false;
    float thc = sqrt(this->_radius*this->_radius - d2);
    t0 = tca - thc;
    t1 = tca + thc;
#else
    // analytic solution
    Vec3f L = ray.origin - this->_center;
    float a = ray.dir.dot(ray.dir);
    float b = 2.f * ray.dir.dot(L);
    float c = L.dot(L) - this->_radius*this->_radius;
    // solve quadratic function
    float discr = b*b - 4.f * a * c;
    if (discr < 0)
        return false;
    else if (discr == 0)
    {
        t0 = -0.5f * b / a;
        t1 = t0;
    }
    else
    {
        float q = (b > 0) ? -0.5f * (b + sqrt(discr)) : -0.5f * (b - sqrt(discr));
        t0 = q / a;
        t1 = c / q;
    }
#endif
    if (t0 > t1)
        std::swap(t0, t1);

    if (t0 < 0)
    {
        t0 = t1; // use t1 if t0 is negative
        if (t0 < 0)
            return false; // both negative
    }

    t = t0;

    ///////////

    return true;
}

Vec3f Sphere::getSurfaceNormal(const Vec3f &p_hit) const
{
	return (p_hit - this->_center).normalize();
}

/**
 * @brief Sphere::getSurfaceColor
 */
Vec3f Sphere::getSurfaceColor(const Vec3f &p_hit) const
{
    return this->_color;
}

PhongCoefficients Sphere::getPhongCoefficients(const Vec3f & p_hit) const
{
	return this->_phongCoeff;
}
