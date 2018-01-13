#include "core.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

void GlfwInstance::init() { static GlfwInstance instance; }

GlfwInstance::GlfwInstance() {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW.");
}

GlfwInstance::~GlfwInstance() { glfwTerminate(); }

namespace
{
	/// Reads a file.
	/// @param file_path Path to the file.
	/// @return Content of the file as string.
	/// @throw std::runtime_error if opening the file fails.
	std::string ReadFile(const char *file_path) {
		std::ifstream file_stream(file_path);
		if (!file_stream.is_open())
			throw std::runtime_error(std::string("Failed to open ") + file_path);
		return std::string(std::istreambuf_iterator<char>(file_stream),
						   std::istreambuf_iterator<char>());
	}

	/// Gets the error log corresponding to a shader.
	/// @param shader Handle of the shader to obtain the error log from.
	/// @return Error log as string.
	std::string GetShaderError(GLuint shader) {
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

	/// Gets the error log corresponding to a shader program.
	/// @param shader Handle of the shader program to obtain the error log from.
	/// @return Error log as string.
	std::string GetProgramError(GLuint program) {
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

	/// Compiles a shader.
	/// @param shader Handle of the shader.
	/// @param file_path Path to the file containing the shader's code.
	/// @throw std::runtime_error if compilation failed.
	void CompileShader(GLuint shader, const char *file_path) {
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

	/// Links a shader program.
	/// @param program Handle of the shader program to link.
	/// @param shaders Handles of the shaders to attach before linking.
	/// @throw std::runtime_error if linking failed.
	void LinkProgram(GLuint program, const std::vector<GLuint> &shaders) {
		// attach shaders & link
		for (auto shader : shaders) {
			glAttachShader(program, shader);
		}
		glLinkProgram(program);

		// check if it succeeded
		GLint link_success;
		glGetProgramiv(program, GL_LINK_STATUS, &link_success);
		if (link_success == 0)
			throw std::runtime_error("Failed to link shader program.\n" + GetProgramError(program));
	}
}

GLuint CreateShaderProgram(const char *vert_path, const char *frag_path) {
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

std::pair<std::vector<Vertex>, std::vector<uint32_t>> LoadObject(const char *file_path) {
	auto ThrowError = [file_path](const char *what) {
		throw std::runtime_error(std::string("Error while parsing ") + file_path + ": " + what);
	};

	std::vector<glm::vec3> in_positions;
	std::vector<glm::vec3> in_normals;
	std::vector<Vertex> out_vertices;
	std::vector<uint32_t> out_indices;
	std::map<std::pair<int, int>, uint32_t> index_map;

	auto GetIndex = [&](int p_index, int n_index) {
		if (p_index < 0 || p_index >= static_cast<int>(in_positions.size()))
			ThrowError(("Index " + std::to_string(p_index) + " out of range").c_str());
		if (n_index < 0 || n_index >= static_cast<int>(in_normals.size()))
			ThrowError(("Index " + std::to_string(n_index) + " out of range").c_str());
		auto iter = index_map.find({ p_index, n_index });
		if (iter != index_map.end())
			return iter->second;
		auto idx = static_cast<uint32_t>(out_vertices.size());
		out_vertices.push_back(Vertex{});
		auto &vertex = out_vertices.back();
		vertex.pos = in_positions[p_index];
		vertex.color = glm::vec3{ 1, 0.15f, 0.15f };
		vertex.normal = in_normals[n_index];
		return idx;
	};

	std::ifstream file_stream(file_path);
	if (!file_stream.is_open())
		throw std::runtime_error(std::string("Failed to open ") + file_path);

	std::string type;
	std::string line;
	std::istringstream line_stream;
	while (std::getline(file_stream, line)) {
		line.erase(line.begin(), std::find_if(line.begin(), line.end(),
			[](char chr) { return !std::isspace(chr); }));
		if (line.empty() || line[0] == '#')
			continue;
		line_stream = std::istringstream(line);
		if (!(line_stream >> type))
			ThrowError(line_stream.str().c_str());
		if (type == "s")
			continue;
		if (type == "v") {
			glm::vec3 pos;
			if (!(line_stream >> pos.x >> pos.y >> pos.z))
				ThrowError(line_stream.str().c_str());
			in_positions.push_back(pos);
		}
		else if (type == "vn") {
			glm::vec3 normal;
			if (!(line_stream >> normal.x >> normal.y >> normal.z))
				ThrowError(line_stream.str().c_str());
			in_normals.push_back(glm::normalize(normal));
		}
		else if (type == "f") {
			std::array<std::string, 3> vals;
			if (!(line_stream >> vals[0] >> vals[1] >> vals[2]))
				ThrowError(line_stream.str().c_str());
			for (const auto &val : vals) {
				auto mid = val.begin() + val.find("//");
				int p_index, n_index;
				if (!(std::istringstream(std::string(val.begin(), mid)) >> p_index) ||
					!(std::istringstream(std::string(mid + 2, val.end())) >> n_index))
					ThrowError(val.c_str());
				out_indices.push_back(GetIndex(p_index - 1, n_index - 1));
			}
		}
		else if (type == "s") {
		}
		else
			ThrowError(type.c_str());

		if (line_stream >> line)
			ThrowError(line.c_str());
		line_stream.clear(std::istream::failbit);
	}
	return{ out_vertices, out_indices };
}

RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const glm::mat4 &model)
{
	RenderBatch batch;
	batch.per_object.model = model;
	batch.index_count = static_cast<int32_t>(indices.size());

	// Create & fill index buffer object
	glGenBuffers(1, &batch.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create vertex array object
	glGenVertexArrays(1, &batch.vao);
	glBindVertexArray(batch.vao);

	// Create & fill vertex buffer object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// Specify & enable position, color and normal attribute (the data is taken from the currently bound vbo)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, pos)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, normal)));

	// Unbind bound objects
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return batch;
}