// Loads OpenGL, GLFW and provides convenience functions.

#pragma once

// OpenGL loader
#include "glad/glad.h"

// OpenGL framework
#include "GLFW/glfw3.h"

#include <vector>

/// RAII class for initializing and terminating GLFW.
/// There shall always be at most one instance of this class.
struct GlfwInstance
{
	GlfwInstance();
	~GlfwInstance();
};

GLuint CreateShaderProgram(const char *vert_path, const char *frag_path);

/// Interpolates linearly in (0, val1), (1, val2).
/// @param x The place to evaluate the interpolation at.
/// @param val1,val2 The values to interpolate.
/// @return The value of the linear interpolation at x.
template <class T> T lerp(float x, T val1, T val2) { return (1 - x) * val1 + x * val2; }
