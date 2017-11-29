#ifndef sceneobject_h
#define sceneobject_h

#define _USE_MATH_DEFINES
#include <memory>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <limits>

#include "util.h"

// Store phong coefficient k_a, k_d, k_s and n in a tuple
typedef std::tuple<Vec3f, Vec3f, Vec3f, float> PhongCoefficients;

/**
 * @brief The SceneObject class.
 */
class SceneObject
{
public:
    /**
     * @brief Construct a scene object with a pseudo random color.
     */
	SceneObject() : _color(getRand(), getRand(), getRand()), _phongCoeff({ _color ,_color ,Vec3f(1.0f), 42.0f}) {}
    /**
     * @brief Destructor
     */
    virtual ~SceneObject() {}

    /**
     * @brief Pure virtual method to compute the intersection of the scene object with a ray.
     * @param ray The ray to check for intersection.
     * @param t Distance on the ray of the intersection.
     * @return true on intersection, false otherwise.
     */
    virtual bool intersect(const Ray &ray, float &t) const = 0;


	virtual Vec3f getSurfaceNormal(const Vec3f &p_hit) const = 0;

    /**
     * @brief Pure virtual method to get the surface color of the SceneObject.
     * @param p_hit The point on the surface that was hit.
     * @return The surface color of the SceneObject.
     */
    virtual Vec3f getSurfaceColor(const Vec3f &p_hit) const = 0;

	/**
	* @brief Pure virtual method to get the surface material properties, i.e. phong coefficients, of the SceneObject.
	* @param p_hit The point on the surface that was hit.
	* @return The phong coefficients of the SceneObject.
	*/
	virtual PhongCoefficients getPhongCoefficients(const Vec3f & p_hit) const = 0;

protected:
    Vec3f _color;   //< color of the scene object
	PhongCoefficients _phongCoeff;
};


/**
 * @brief The Plane class.
 *        A plane is represented by a point on the plane and a normal.
 */
class Plane : public SceneObject
{
public:
    Plane(const Vec3f &point, const Vec3f &normal) : _point(point), _normal(normal) {}

    bool intersect(const Ray &ray, float &t) const override;

	Vec3f getSurfaceNormal(const Vec3f &p_hit) const override;

    Vec3f getSurfaceColor(const Vec3f &p_hit) const override;

	PhongCoefficients getPhongCoefficients(const Vec3f & p_hit) const override;

    Vec3f _point;   //< Point on the plane.
    Vec3f _normal;  //< Normal of the plane.
};


/**
 * @brief The Sphere class.
 *        A sphere is represented implicitly by a center and a radius.
 */
class Sphere : public SceneObject
{
public:
    Sphere(const Vec3f &center, const float &radius) : _radius(radius), _center(center) {}

    bool intersect(const Ray &ray, float &t) const override;

	Vec3f getSurfaceNormal(const Vec3f &p_hit) const override;

    Vec3f getSurfaceColor(const Vec3f &p_hit) const override;

	PhongCoefficients getPhongCoefficients(const Vec3f & p_hit) const override;

    float _radius;  //< Radius of the sphere.
    Vec3f _center;  //< Center of the sphere.
};


#endif // !sceneobject_h
