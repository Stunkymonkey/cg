#include <iostream>

#include "sceneobject.h"
#include "pointlight.h"

// random number generation
std::uniform_real_distribution<> distrib(-0.5, 0.5);
const static int SEED = 42;

const static int WIDTH = 1024;
const static int HEIGHT = 1024;
const static int MAX_DEPTH = 5;

//////////
// TODO 2:
// Compute Phong lighting
//
Vec3f computePhongLighting(
	Vec3f const& view_direction,			//< direction from surface point to camera origin
	Vec3f const& surface_normal,			//< normal vector at surface point
	Vec3f const& light_direction,			//< direction from surface point to light source
	PhongCoefficients const& phong_coeff,	//< phong coefficient k_a,k_d,k_s and n
	Vec3f const& light_color,				//< color of the light source
	float light_intensity)					//< intensity of the light source
{
	/*TODO: replace*/ return Vec3f(); /*TODO: replace*/
}

/**
 * @brief Method to check a ray for intersections with any object of the scene.
 * @param ray The ray to trace.
 * @param objects Vector of pointers to all scene objects.
 * @param t_near The intersection distance from the ray origin to the closest point hit.
 * @param hitObject The closest object hit.
 * @return true on hit, false otherwise
 */
bool trace(const Ray &ray,
           const std::vector<std::unique_ptr<SceneObject>> &objects,
           float &t_near, const SceneObject *&hitObject)
{
    t_near = std::numeric_limits<float>::max();

    // Check all objects if they got hit by the traced ray. (cf. lecture slide 54)
    // If any object got hit, return the one closest to the camera as 'hitObject'.
    for (auto &o : objects)
    {
        float t = std::numeric_limits<float>::max();
        if (o->intersect(ray, t) && t < t_near)
        {
            hitObject = o.get();
            t_near = t;
        }
    }

    return (hitObject != nullptr);
}

/**
 * @brief Cast a ray into the scene. If the ray hits at least one object,
 *        the color of the object closest to the camera is returned.
 * @param ray The ray that's being cast.
 * @param objects All scene objects.
 * @return The color of a hit object that is closest to the camera.
 *         Return dark blue if no object was hit.
 */
Vec3f castRay(const Ray &ray, const std::vector<std::unique_ptr<SceneObject>> &objects, const std::vector<Pointlight> & lights)
{
    // set the background color as dark blue
    Vec3f hitColor(0, 0, 0.2f);
	// early exit if maximum recursive depth is reached - return background color
	if (ray.depth > MAX_DEPTH)
		return hitColor;
    // pointer to the object that was hit by the ray
    const SceneObject *hitObject = nullptr;
    // intersection distance from the ray origin to the point hit
    float t = std::numeric_limits<float>::max();

    // Trace the ray. If an object gets hit, calculate the hit point and
    // retrieve the surface color 'hitColor' from the 'hitObject' object that was hit
    if (trace(ray, objects, t, hitObject))
    {
		hitColor = Vec3f(0.0f);

		// Intersection point with the hit object
        Vec3f p_hit = ray.origin + ray.dir * t;

		//////////
		// TODO 3:
		// Compute local lighting. The result is added to "hitColor".
		//
		// For each light source (given by funtion parameter lights)
		//
		//   a) Cast a shadow ray from the hitpoint to the light-source (use the trace function)
		//
		//   b) If not in shadow, compute local lighting using the function "computePhongLighting"
		//		Else apply ambient term only
		//
		//		For a more realistic image, use inverse square attentuation for the light intensity.
		//

		/*TODO: replace*/ hitColor = hitObject->getSurfaceColor(p_hit); /*TODO: replace*/

		//////////
		// TODO 4:
		// Compute reflection.
		//
		// Build a reflection for the hitpoint of the hit object. 
		// Use this ray to make a recursive call to "castRay" and add the result of the call to "hitColor".
		//
    }

    return hitColor;
}

/**
 * @brief The rendering method, loop over all pixels in the framebuffer, shooting
 *        a ray through each pixel with the origing being the camera position.
 * @param viewport Size of the framebuffer.
 * @param objects Vector of pointers to all objects contained in the scene.
 */
void render(const Vec2i viewport, const std::vector<std::unique_ptr<SceneObject>> &objects, const std::vector<Pointlight> & lights)
{
    std::vector<Vec3f> framebuffer(viewport.x * viewport.y);

    // camera position in world coordinates
    const Vec3f cameraPos(0.f, 0.f, -1.f);
    // view plane parameters
    const float l = -1.f;   // left
    const float r = +1.f;   // right
    const float b = -1.f;   // bottom
    const float t = +1.f;   // top
    const float d = +2.f;   // distance to camera

    // Cast a ray from 'cameraPos' through the center(!) of each pixel on the viewplane.
    // Use the view plane parametrization given above (l,r,b,t,d).
    Ray ray;
    ray.origin = cameraPos;
    int id = 0;
    for (int j = 0; j < viewport.y; ++j)
    {
        for (int i = 0; i < viewport.x; ++i)
        {
            float u = l + (r-l) * (i + 0.5f) / viewport.x;
            float v = b + (t-b) * (j + 0.5f) / viewport.y;

            ray.dir = Vec3f(u, v, d);
			ray.dir = ray.dir.normalize();
            framebuffer.at(id++) = castRay(ray, objects, lights);
        }
    }

    // save the framebuffer an a PPM image
    saveAsPPM("./result.ppm", viewport, framebuffer);
}

/**
 * @brief main routine.
 *        Generates the scene and invokes the rendering.
 * @return
 */
int main()
{
    std::vector<std::unique_ptr<SceneObject>> objects;
	std::vector<Pointlight> pointlights;

    // random number generation
    std::mt19937 mtGen(SEED);
    std::uniform_real_distribution<> distrib(-0.5, 0.5);

    // generate a scene containing one plane, a bunch of pseudo randomly distributed spheres
	// and a bunch of pseudo randomly distributed pointlights
    Vec3f planeNormal(0.0f, 1.f, 0.0f);
    planeNormal.normalize();
    objects.push_back(std::unique_ptr<SceneObject>(new Plane(Vec3f(0.f, -1.f, 5.f), planeNormal)));

    const int numSpheres = 32;
    for (int i = 0; i < numSpheres; ++i)
    {
        const Vec3f randPos(distrib(mtGen)*10.f, distrib(mtGen)*10.f, 12.f + distrib(mtGen)*10.f);
        const float randRadius = (0.5f + distrib(mtGen));
        objects.push_back(std::unique_ptr<SceneObject>(new Sphere(randPos, randRadius)));
    }

	const int numLights = 16;
	for (int i = 0; i < numLights; ++i)
	{
		const Vec3f randPos(distrib(mtGen)*30.f, 10.0f + distrib(mtGen)*20.f, 10.0f + distrib(mtGen)*30.f);
		pointlights.push_back(Pointlight(randPos));
	}

    const Vec2i viewport(WIDTH, HEIGHT);
    render(viewport, objects, pointlights);

    return 0;
}
