#ifndef pointlight_h
#define pointlight_h

#define _USE_MATH_DEFINES
#include <memory>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <limits>

#include "util.h"


/**
* @brief The Pointlight class.
*/
class Pointlight
{
public:
	/**
	* @brief Construct a point light with a pseudo random color and intensity.
	*/
	Pointlight() : _position(0.0f,0.0f,0.0f), _color(getRand(), getRand(), getRand()), _intensity(2.0f + getRand()*1.5f) {}

	/**
	* @brief Construct a point light with a pseudo random color and intensity, located at a given position in the world.
	*/
	Pointlight(Vec3f position) : _position(position), _color(getRand(), getRand(), getRand()), _intensity(2.0f + getRand()*1.5f) {}

	/**
	* @brief Get the position of the point light.
	* @return The position of the light source.
	*/
	Vec3f getPosition() const { return _position; };

	/**
	* @brief Get the color of the point light.
	* @return The color of the light source.
	*/
	Vec3f getColor() const { return _color; };

	/**
	* @brief Get the intensity of the point light.
	* @return The intensity of the light source given in Lume.
	*/
	float getIntensity() const { return _intensity; };

private:
	Vec3f _position;	//< position of the light source

	Vec3f _color;		//< color of the light source
	float _intensity;	//< intensity of the light source
};


#endif // !pointlight_h
