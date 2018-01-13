#pragma once

#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

#include <array>
#include <utility>
#include <vector>

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
	glm::mat4 view;
	glm::mat4 projection;

	glm::ivec2 resolution;
	float aspect;
	float fov;

	int render_mode;
};

/// Layout of the per-object data submitted to the graphics device.
struct PerObject
{
	glm::mat4 model;
};

/// Layout of the constant data submitted to the graphics device.
struct ConstantBuffer
{
	std::array<glm::vec4, 16> reflections;
	std::array<glm::vec4, 8> kernel;
};

/// Combination of a vertex array object, an index buffer object and the
/// per-object data used to render it.
struct RenderBatch
{
	GLuint vao;
	GLuint ibo;

	int32_t index_count;

	PerObject per_object;
};

/// Render mode
enum class RenderMode
{
	COLOR_ONLY = 1,
	COLOR_AND_SSAO = 2,
	BLINN_PHONG = 3,
	BLINN_PHONG_AND_SSAO = 4,
	SSAO_ONLY = 5
};

/// RAII class for initializing and terminating GLFW.
/// This is a singleton since GLFW should be initialized exactly once.
class GlfwInstance
{
  public:
    /// Ensures that GLFW is initialized.
    static void init();

  private:
    /// Constructor initializes GLFW.
    GlfwInstance();

    /// Destructor terminates GLFW.
    ~GlfwInstance();
};

/// Creates a shader program.
/// @param vert_path Path to the file containing the vertex shader's code.
/// @param frag_path Path to the file containing the fragment shader's code.
/// @return Handle to the created shader program.
GLuint CreateShaderProgram(const char *vert_path, const char *frag_path);

/// Load an object.
/// @param file_path Path to the file containing the object.
/// @return Vertex and index buffer data.
std::pair<std::vector<Vertex>, std::vector<uint32_t>> LoadObject(const char *file_path);

/// Create render batch.
/// @param vertices A vertex buffer.
/// @param out_indices An index buffer.
/// @return RenderBatch object created using the given buffers.
RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const glm::mat4 &model = glm::mat4{});

/// Interpolates linearly in (0, val1), (1, val2).
/// @param x The place to evaluate the interpolation at.
/// @param val1,val2 The values to interpolate.
/// @return The value of the linear interpolation at x.
template <class T> T lerp(float x, T val1, T val2)
{
	return (1 - x) * val1 + x * val2;
}
