#include <core.hpp>

#include <fstream>
#include <string>

/// Constructor initializes GLFW.
GlfwInstance::GlfwInstance()
{
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW.");
}

/// Destructor terminates GLFW.
GlfwInstance::~GlfwInstance() { glfwTerminate(); }

/// Reads a file.
/// @param file_path Path to the file.
/// @return Content of the file as string.
/// @throw std::runtime_error if opening the file fails.
std::string ReadFile(const char *file_path)
{
	std::ifstream file_stream(file_path);
	if (!file_stream.is_open())
		throw std::runtime_error(std::string("Failed to open ") + file_path);
	return std::string(std::istreambuf_iterator<char>(file_stream),
	                   std::istreambuf_iterator<char>());
}

/// Gets the error log corresponding to a shader.
/// @param shader Handle of the shader to obtain the error log from.
/// @return Error log as string.
std::string GetShaderError(GLuint shader)
{
	// get error log length
	GLint info_log_length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length == 0)
		return std::string();

	// copy error log into a buffer
	std::vector<GLchar> info_log(info_log_length);
	glGetShaderInfoLog(shader, info_log_length, nullptr, info_log.data());
	return std::string(info_log.data());
}

/// Compiles a shader.
/// @param shader Handle of the shader.
/// @param file_path Path to the file containing the shader's code.
/// @throw std::runtime_error if compilation failed.
void CompileShader(GLuint shader, const char *file_path)
{
	// compile
	std::string code = ReadFile(file_path);
	const char *p_code = code.c_str();
	glShaderSource(shader, 1, &p_code, nullptr);
	glCompileShader(shader);

	// check if compiling succeeded
	GLint compile_success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
	if (compile_success == 0)
		throw std::runtime_error(std::string("Failed to compile shader \"") + file_path + "\".\n" +
		                         GetShaderError(shader));
}

/// Gets the error log corresponding to a shader program.
/// @param shader Handle of the shader program to obtain the error log from.
/// @return Error log as string.
std::string GetProgramError(GLuint program)
{
	// get error log length
	GLint info_log_length;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length == 0)
		return std::string();

	// copy error log into a buffer
	std::vector<GLchar> info_log(info_log_length);
	glGetProgramInfoLog(program, info_log_length, nullptr, info_log.data());
	return std::string(info_log.data());
}

/// Links a shader program.
/// @param program Handle of the shader program to link.
/// @param shaders Handles of the shaders to attach before linking.
/// @throw std::runtime_error if linking failed.
void LinkProgram(GLuint program, const std::vector<GLuint> &shaders)
{
	// attach shaders & link
	for (auto shader : shaders)
	{
		glAttachShader(program, shader);
	}
	glLinkProgram(program);

	// check if it succeeded
	GLint link_success;
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);
	if (link_success == 0)
		throw std::runtime_error("Failed to link shader program.\n" + GetProgramError(program));
}

/// Creates a shader program.
/// @param vert_path Path to the file containing the vertex shader's code.
/// @param frag_path Path to the file containing the fragment shader's code.
/// @return Handle to the created shader program.
GLuint CreateShaderProgram(const char *vert_path, const char *frag_path)
{
	// compile vertex & fragment shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	CompileShader(vertex_shader, vert_path);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	CompileShader(fragment_shader, frag_path);

	// link shaders
	GLuint shader_program = glCreateProgram();
	LinkProgram(shader_program, {vertex_shader, fragment_shader});

	// clean shaders up
	glDetachShader(shader_program, fragment_shader);
	glDetachShader(shader_program, vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);

	return shader_program;
}
