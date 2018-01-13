#pragma once

#include "core.hpp"

namespace cg
{
	class SSAO_Renderer
	{
	public:
		/// Run the renderer until user exits.
		/// @return Error code.
		int Run();

	private:
		/// Creates the window and sets persistent render settings and compiles shaders.
		/// @throw std::runtime_error If an ThrowError occurs during intialization.
		void InitRenderer();

		/// Creates geometry and uniform buffers.
		void InitScene();

		/// Generates the sample kernel for SSAO.
		/// @return The created ConstantBuffer object
		ConstantBuffer GenSampleKernel();

		/// Update the frame buffers.
		void UpdateFramebuffers();

		/// Loads the shader programs or reloads them.
		void UpdateShaders();

		/// Synchronizes scene updates to a frequenzy of 100 Hz.
		/// This function assumes that the scene is updated each time it returns true.
		/// @return true iff the scene should be updated now.
		bool NeedSceneUpdate();

		/// Applies per-frame updates of the scene.
		void UpdateScene();

		/// Renders a frame.
		void RenderFrame();

		/// Renders a RenderBatch.
		/// @param batch The RenderBatch to render.
		void RenderObject(const RenderBatch &batch);

		/// Updates the resolution. This is called by GLFW when the resolution changes.
		/// @param window The window that changed resolution.
		/// @param width New width.
		/// @param height New height.
		void FramebufferSizeCallback(GLFWwindow *window, int width, int height);

		/// Reloads the shaders if 'R' is pressed. This is called by GLFW when a key is pressed.
		/// @param window The window in which a key was pressed.
		/// @param key GLFW key code of the pressed key.
		/// @param scancode platform-specific key code.
		/// @param action One of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
		/// @param mods Modifier bits.
		void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

		/// Helper function for controlling an axis with 2 keys.
		/// @param window The window where the keys should be pressed.
		/// @param key1 The first key.
		/// @param key2 The second key.
		/// @return -1 if only the first key is pressed, 1 if only the second key is pressed and 0 if none of the keys or both keys are pressed.
		int KeyAxisValue(GLFWwindow *window, int key1, int key2);

		/// Helper function for number key pressed event.
		/// @param window The window where the keys should be pressed.
		/// @return Numerical value of the pressed key (1-5).
		void NumberKeyPressed(GLFWwindow *window);

		// Window information
		GLFWwindow *window;
		int window_width = 960, window_height = 540;

		// Shaders for the three render passes
		GLuint shader_program_geometry = 0;
		GLuint shader_program_ao = 0;
		GLuint shader_program_final = 0;

		// Color and frame buffers for the 1st render pass
		GLuint tex_color;
		GLuint tex_normal_depth;
		GLuint tex_geometry_depth;
		GLuint framebuffer_geometry;

		// Color and frame buffers for the 2nd render pass
		GLuint tex_ao;
		GLuint framebuffer_ao;

		// Empty vertex array object, used for rendering a full screen rectangle
		GLuint empty_vao;

		// Uniform buffers containing per-frame or per-object data
		GLuint ubo_per_frame;
		GLuint ubo_per_object;

		// Local copy of the per-frame uniform buffer
		PerFrame per_frame;

		// Geometry of the ground and the drake
		RenderBatch render_batch_ground;
		RenderBatch render_batch_drake;

		// Camera settings
		glm::vec3 cam_pos{ 18.0, 5.0, 21.0 };
		glm::vec3 cam_velocity{ 0.0, 0.0, 0.0 };

		float cam_yaw = 0.53f;
		float diff_cam_yaw = 0.0f;
		float cam_pitch = 0.02f;
		float diff_cam_pitch = 0.0f;

		// Render mode
		RenderMode render_mode = RenderMode::BLINN_PHONG;
	};
}