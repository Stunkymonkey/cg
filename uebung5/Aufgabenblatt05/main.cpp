#include <iostream>

#include "sceneobject.h"

// TODO set the test according to your current exercise.
#define TEST_RAY_GENERATION 1
#define TEST_SPHERE_INTERSECT 0

// random number generation
std::uniform_real_distribution<> distrib(-0.5, 0.5);
const static int SEED = 42;

const static int WIDTH = 600;
const static int HEIGHT = 600;

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

    /////////////
    // TODO 2.1.3
    // Check all objects if they got hit by the traced ray.
    // If any object got hit, return the one closest to the camera as 'hitObject'.
    // cf. lecture slide 54
    //
    // END TODO
    /////////////

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
Vec3f castRay(const Ray &ray, const std::vector<std::unique_ptr<SceneObject>> &objects)
{
    // set the background color as dark blue
    Vec3f hitColor(0, 0, 0.2f);
    // pointer to the object that was hit by the ray
    const SceneObject *hitObject = nullptr;
    // intersection distance from the ray origin to the point hit
    float t = std::numeric_limits<float>::max();

    //////////////
    // TODO: 2.1.2
    // Trace the ray by calling 'trace(...)'. If an object gets hit, calculate the hit point
    // and retrieve the surface color 'hitColor' from the 'hitObject'.
    // cf. lecture slide 54
    //
    // END TODO
    //////////////

    return hitColor;
}

/**
 * @brief The rendering method, loop over all pixels in the framebuffer, shooting
 *        a ray through each pixel with the origing being the camera position.
 * @param viewport Size of the framebuffer.
 * @param objects Vector of pointers to all objects contained in the scene.
 */
void render(const Vec2i viewport, const std::vector<std::unique_ptr<SceneObject>> &objects)
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

    //////////////
    // TODO: 2.1.1
    // Cast a ray from 'cameraPos' through the center(!) of each pixel on the view plane.
    // Use the view plane parametrization given above (l, r, b, t and d).
    // cf. lecture slides 39-43
    //
    // END TODO
    //////////////

    // save the framebuffer an a PPM image
    saveAsPPM("./result.ppm", viewport, framebuffer);

    // Compare the resulting image to the reference images.
    // Enable the test according to your current exercise.
	// Depending on your build/execution path you may have to change the given path!
#if TEST_RAY_GENERATION
    // Check your ray generation and setup against the reference.
    comparePPM("./test/reference_rayGeneration.ppm",
               "ray generation test", framebuffer);
#elif TEST_SPHERE_INTERSECT
    // Check your ray-sphere intersection against the reference.
    comparePPM("./test/reference_sphereIntersection.ppm",
               "sphere intersection test", framebuffer);
#endif
}

/**
 * @brief main routine.
 *        Generates the scene and invokes the rendering.
 * @return
 */
int main()
{
    std::vector<std::unique_ptr<SceneObject>> objects;

    // random number generation
    std::mt19937 mtGen(SEED);
    std::uniform_real_distribution<> distrib(-0.5, 0.5);

    // generate a scene containing one plane and a bunch of pseudo randomly distributed spheres
    Vec3f planeNormal(0.f, -1.f, 0.1f);
    planeNormal.normalize();
    objects.push_back(std::unique_ptr<SceneObject>(new Plane(Vec3f(0.f, 0.f, 50.f), planeNormal)));

    const int numSpheres = 32;
    for (int i = 0; i < numSpheres; ++i)
    {
        const Vec3f randPos(distrib(mtGen)*10.f, distrib(mtGen)*10.f, 12.f + distrib(mtGen)*10.f);
        const float randRadius = (0.5f + distrib(mtGen));
        objects.push_back(std::unique_ptr<SceneObject>(new Sphere(randPos, randRadius)));
    }

    const Vec2i viewport(WIDTH, HEIGHT);
    render(viewport, objects);

    return 0;
}
