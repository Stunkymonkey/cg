// Loads OpenGL, GLFW and provides convenience functions.

#pragma once

// OpenGL loader
#include "glad/glad.h"

// OpenGL framework
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>

#include <vector>

/// RAII class for initializing and terminating GLFW.
/// There shall always be at most one instance of this class.
struct GlfwInstance
{
	GlfwInstance();
	~GlfwInstance();
};

/// Layout of the vertices, used to store them in vertex buffer objects.
struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
};

/// Layout of the per-frame data submitted to the graphics device.
struct PerFrame
{
	glm::mat4 view;                     // view matrix
	glm::mat4 proj;                     // projection matrix
	glm::mat4 inv_view_proj;            // clip coords -> world coords
	glm::mat4 shadowmap_view_proj;      // world coords -> shadow map's clip coords
	glm::mat4 clip_coords_to_shadowmap; // clip coords -> shadow map's clip coords
	glm::vec4 light_dir;                // light direction
	glm::ivec2 resolution;              // window resolution
	glm::ivec2 shadowmap_resolution;    // shadow map resolution
};

/// Layout of the per-object data submitted to the graphics device.
struct PerObject
{
	glm::mat4 model;  // object coords -> world coords
	int use_texture; // true if tex_ground shall be sampled for obtaining the color
};

/// Combination of a vertex array object, an index buffer object and the per-object data used to
/// render it.
struct RenderBatch
{
	GLuint vao; // vertex array object
	GLuint ibo; // index buffer object
	int32_t index_count;
	PerObject per_object;
};

GLuint CreateShaderProgram(const char *vert_path, const char *frag_path);
int KeyAxisValue(GLFWwindow *window, int key1, int key2);
std::pair<std::vector<Vertex>, std::vector<uint32_t>> LoadObject(const char *filename);

/// Interpolates linearly in (0, val1), (1, val2).
/// @param x The place to evaluate the interpolation at.
/// @param val1,val2 The values to interpolate.
/// @return The value of the linear interpolation at x.
template <class T> T lerp(float x, T val1, T val2) { return (1 - x) * val1 + x * val2; }
