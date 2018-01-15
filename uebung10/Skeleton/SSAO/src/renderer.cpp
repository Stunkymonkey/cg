#include "renderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

int cg::SSAO_Renderer::Run()
{
	try
	{
		// Initialize
		GlfwInstance::init();

		InitRenderer();
		InitScene();

		// Render until the window should close
		while (!glfwWindowShouldClose(this->window))
		{
			glfwPollEvents();

			while (NeedSceneUpdate())
			{
				UpdateScene();
			}

			RenderFrame();
			glfwSwapBuffers(this->window);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "ERROR: " << e.what() << "\nPress enter to finish.\n";
		getchar();
		return -1;
	}

	return 0;
}

void cg::SSAO_Renderer::InitRenderer()
{
	// Create window
	this->window = glfwCreateWindow(this->window_width, this->window_height, "SSAO", nullptr, nullptr);

	if (!this->window)
	{
		throw std::runtime_error("Failed to create window.");
	}

	// Use the window and get the resolution
	glfwMakeContextCurrent(this->window);
	glfwGetFramebufferSize(this->window, &this->window_width, &this->window_height);

	// Set a callbacks for resolution changes and key strokes
	glfwSetWindowUserPointer(this->window, this);

	auto framebufferSizeCallback = [](GLFWwindow* window, int width, int height)
	{
		static_cast<SSAO_Renderer*>(glfwGetWindowUserPointer(window))->FramebufferSizeCallback(window, width, height);
	};

	auto keyCallback = [](GLFWwindow* window, int key, int scanconde, int action, int mods)
	{
		static_cast<SSAO_Renderer*>(glfwGetWindowUserPointer(window))->KeyCallback(window, key, scanconde, action, mods);
	};

	glfwSetFramebufferSizeCallback(this->window, framebufferSizeCallback);
	glfwSetKeyCallback(this->window, keyCallback);
	
	// Enable vsync
	glfwSwapInterval(1);

	// Load OpenGL
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		throw std::runtime_error("Failed to load OpenGL context.");
	}

	std::cout << "OpenGL " << glGetString(GL_VERSION) << '\n';
	std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	// Enable depth buffering and back-face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Create shader programs
	UpdateShaders();

	// Create the per-frame uniform buffer and bind it to position 0 for all shaders (will be filled later, once per frame)
	glGenBuffers(1, &this->ubo_per_frame);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_per_frame);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrame), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->ubo_per_frame);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Create the per-object uniform buffer and bind it to position 1 for all shaders (will be filled later, once per rendering an object)
	glGenBuffers(1, &this->ubo_per_object);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_per_object);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerObject), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->ubo_per_object);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Create & fill the constant uniform buffer and bind it to position 2 for all shaders
	GLuint ubo_constants;
	glGenBuffers(1, &ubo_constants);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_constants);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_constants);

	void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	ConstantBuffer constants = GenSampleKernel();
	memcpy(data, &constants, sizeof(constants));

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Create an empty vertex array object, used for rendering a full screen rectangle
	glGenVertexArrays(1, &this->empty_vao);

	// Create the intermediate framebuffer to render the geometry to
	glGenTextures(1, &this->tex_color);
	glGenTextures(1, &this->tex_normal_depth);
	glGenTextures(1, &this->tex_geometry_depth);
	glGenFramebuffers(1, &this->framebuffer_geometry);

	// Create the intermediate framebuffer to render the ambient occlusion values to
	glGenTextures(1, &this->tex_ao);
	glGenFramebuffers(1, &this->framebuffer_ao);

	UpdateFramebuffers();
}

void cg::SSAO_Renderer::InitScene()
{
	// Create ground geometry
	std::vector<Vertex> ground_vertices{
		{ { -20.5f, 0.0f, -20.5f },{ 0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { -20.5f, 0.0f, 20.5f },{ 0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 20.5f, 0.0f, 20.5f },{ 0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 20.5f, 0.0f, -20.5f },{ 0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } } };

	std::vector<uint32_t> ground_indices{ 0, 1, 2, 0, 2, 3 };

	this->render_batch_ground = CreateRenderBatch(ground_vertices, ground_indices);

	// Create the drake's geometry
	std::vector<Vertex> drake_vertices;
	std::vector<uint32_t> drake_indices;

	std::tie(drake_vertices, drake_indices) = LoadObject("objects/dragon_small.obj");

	glm::mat4 transform;
	transform = glm::translate(transform, glm::vec3{ 0, 6, 0 });
	transform = glm::scale(transform, glm::vec3{ 0.15f });

	this->render_batch_drake = CreateRenderBatch(drake_vertices, drake_indices, transform);
}

ConstantBuffer cg::SSAO_Renderer::GenSampleKernel()
{
	ConstantBuffer result;

	auto dist = std::normal_distribution<float>();
	auto engine = std::default_random_engine();

	// Create random samples within a sphere
	for (size_t i = 0; i < result.kernel.size(); ++i)
	{
		glm::vec3 random_vec(dist(engine), dist(engine), dist(engine));

		float scale = static_cast<float>(i) / result.kernel.size();
		scale = lerp(scale * scale, 0.05f, 1.0f);

		result.kernel[i] = glm::vec4(glm::normalize(random_vec) * scale, 0.0f);
	}

	// Create noise on a sphere
	for (size_t i = 0; i < result.reflections.size(); ++i) {
		glm::vec3 random_vec(dist(engine), dist(engine), dist(engine));

		result.reflections[i] = glm::vec4(glm::normalize(random_vec), 0.0f);
	}

	return result;
}

void cg::SSAO_Renderer::UpdateFramebuffers()
{
	// Set viewport
	glViewport(0, 0, this->window_width, this->window_height);

	// Update the texture for rendering the color values to
	glBindTexture(GL_TEXTURE_2D, this->tex_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->window_width, this->window_height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Update the texture for rendering the normals and depth values to
	glBindTexture(GL_TEXTURE_2D, this->tex_normal_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, this->window_width, this->window_height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Update the depth buffer used for rendering the geometry
	glBindTexture(GL_TEXTURE_2D, this->tex_geometry_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, this->window_width, this->window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Update the framebuffer for the 1st render pass where the geometry is rendered
	///////
	// TODO
	// Bind the geometry framebuffer. Then use glFramebufferTexture2D to attach the color texture as the first,
	// the normal-depth texture as the second color attachment and the geometry depth texture as depth attachment.
	// Now check the success of this operation using the glCheckFramebufferStatus function and throw a
	// runtime error when it is not set to complete. Finally, unbind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_geometry);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_normal_depth, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_geometry_depth, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("framebuffer error");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Update the texture for rendering the ambient occlusion values to
	glBindTexture(GL_TEXTURE_2D, this->tex_ao);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, this->window_width, this->window_height, 0, GL_RED, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Update the framebuffer for the 2nd render pass where the ambient occlusion values are calculated
	///////
	// TODO
	// Bind the ambient occlusion framebuffer. Then use glFramebufferTexture2D to attach the ambient occlusion texture
	// as the first color attachment. Now check the operation of this implementation using the glCheckFramebufferStatus
	// function and throw a runtime error when it is not set to complete. Finally, unbind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_ao);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_ao, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("aobuffer error");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cg::SSAO_Renderer::UpdateShaders()
{
	// Remove previous version of the shaders
	glDeleteProgram(this->shader_program_geometry);
	glDeleteProgram(this->shader_program_ao);
	glDeleteProgram(this->shader_program_final);

	try
	{
		// Load shaders into programs
		this->shader_program_geometry = CreateShaderProgram("shaders/geometry_vert.glsl", "shaders/geometry_frag.glsl");
		this->shader_program_ao = CreateShaderProgram("shaders/quad_vert.glsl", "shaders/ao_frag.glsl");
		this->shader_program_final = CreateShaderProgram("shaders/quad_vert.glsl", "shaders/final_frag.glsl");
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << "\nPress 'R' to reload the shaders.\n";
	}
}

bool cg::SSAO_Renderer::NeedSceneUpdate()
{
	// Initial next update value
	static auto nextUpdate = std::chrono::high_resolution_clock::now();

	// Check time
	auto now = std::chrono::high_resolution_clock::now();

	if (nextUpdate <= now)
	{
		nextUpdate += std::chrono::milliseconds{ 10 };
		return true;
	}

	return false;
}

void cg::SSAO_Renderer::UpdateScene()
{
	// Update render mode (determines whether to render color, ambient occlusion, or both, or Blinn-Phong in the shader)
	NumberKeyPressed(this->window);

	// Update camera yaw and pitch
	const int rot_horz_target = KeyAxisValue(this->window, GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
	const int rot_vert_target = KeyAxisValue(this->window, GLFW_KEY_DOWN, GLFW_KEY_UP);

	this->diff_cam_yaw = lerp(0.1f, this->diff_cam_yaw, static_cast<float>(rot_horz_target));
	this->diff_cam_pitch = lerp(0.1f, this->diff_cam_pitch, static_cast<float>(rot_vert_target));

	this->cam_yaw += 0.03f * this->diff_cam_yaw;
	this->cam_yaw = std::remainder(this->cam_yaw, 2 * glm::pi<float>());

	this->cam_pitch += 0.03f * this->diff_cam_pitch;
	this->cam_pitch = glm::clamp(this->cam_pitch, -1.5f, 1.5f);

	// Calculate camera direction
	glm::vec3 cam_dir{ 0, 0, -1 };
	cam_dir = glm::mat3{ glm::rotate(glm::mat4(), this->cam_pitch, glm::vec3{ 1, 0, 0 }) } *cam_dir;
	cam_dir = glm::mat3{ glm::rotate(glm::mat4(), this->cam_yaw, glm::vec3{ 0, 1, 0 }) } *cam_dir;

	// Calculate the rotation matrix used to orient the target velocity along the camera
	glm::vec2 cam_dir_2d{ cam_dir.x, cam_dir.z };
	cam_dir_2d = glm::normalize(cam_dir_2d);

	const glm::mat3 rot{ -cam_dir_2d.y, 0, cam_dir_2d.x, 0, 1, 0, -cam_dir_2d.x, 0, -cam_dir_2d.y };

	// Update camera velocity and position
	glm::vec3 target_velocity = rot * glm::vec3{
		KeyAxisValue(this->window, GLFW_KEY_A, GLFW_KEY_D),
		KeyAxisValue(this->window, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE),
		KeyAxisValue(this->window, GLFW_KEY_W, GLFW_KEY_S) };

	this->cam_velocity = lerp(0.08f, this->cam_velocity, target_velocity);
	this->cam_pos += 0.05f * this->cam_velocity;

	// Calculate view matrix
	glm::mat4 view = glm::lookAt(this->cam_pos, this->cam_pos + cam_dir, glm::vec3{ 0, 1, 0 });

	// Calculate projection matrix
	const float aspect = static_cast<float>(this->window_width) / this->window_height;

	glm::mat4 projection = glm::perspective(0.25f * glm::pi<float>(), aspect, 0.1f, 100.f);

	// Update the local copy of the per-frame uniform buffer
	this->per_frame.view = view;
	this->per_frame.projection = projection;
	this->per_frame.resolution = { this->window_width, this->window_height };
	this->per_frame.aspect = aspect;
	this->per_frame.fov = 0.25f * glm::pi<float>();
	this->per_frame.render_mode = static_cast<int>(this->render_mode);
}

void cg::SSAO_Renderer::RenderFrame()
{
	// Update per-frame uniform buffer
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_per_frame);

	void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(data, &this->per_frame, sizeof(this->per_frame));

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Setup for rendering the geometry
	std::array<GLenum, 2> render_targets_geometry{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_geometry);
	glDrawBuffers(static_cast<int>(render_targets_geometry.size()), render_targets_geometry.data());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(this->shader_program_geometry);

	// Render the geometry
	RenderObject(this->render_batch_ground);
	RenderObject(this->render_batch_drake);

	// Setup for rendering the ao values
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_ao);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(this->shader_program_ao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->tex_color);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->tex_normal_depth);

	// Render a full screen quad
	glBindVertexArray(this->empty_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// Setup for rendering to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(this->shader_program_final);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->tex_color);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->tex_normal_depth);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->tex_ao);

	// Render a full screen quad
	glBindVertexArray(this->empty_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUseProgram(0);
}

void cg::SSAO_Renderer::RenderObject(const RenderBatch &batch)
{
	// Upload per-object data
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_per_object);

	void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(data, &batch.per_object, sizeof(batch.per_object));

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Actually render
	glBindVertexArray(batch.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ibo);
	glDrawElements(GL_TRIANGLES, batch.index_count, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void cg::SSAO_Renderer::FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	if (width > 0 && height > 0)
	{
		this->window_width = width;
		this->window_height = height;

		UpdateFramebuffers();
	}
}

void cg::SSAO_Renderer::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		UpdateShaders();
	}
}

int cg::SSAO_Renderer::KeyAxisValue(GLFWwindow *window, int key1, int key2)
{
	bool key1_pressed = glfwGetKey(this->window, key1) == GLFW_PRESS;
	bool key2_pressed = glfwGetKey(this->window, key2) == GLFW_PRESS;

	return key2_pressed - key1_pressed;
}

void cg::SSAO_Renderer::NumberKeyPressed(GLFWwindow *window)
{
	const static std::array<int, 10> keys = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5 };

	for (const auto& key : keys)
	{
		if (glfwGetKey(this->window, key) == GLFW_PRESS)
		{
			this->render_mode = static_cast<RenderMode>(key - GLFW_KEY_0);
		}
	}
}