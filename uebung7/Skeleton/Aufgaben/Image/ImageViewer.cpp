#include "ImageViewer.h"

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw_gl3.h"

#include <GLFW/glfw3.h>

#include "../Scene/Objects/Container.h"

namespace cg
{
	namespace
	{
		/** Check for existing file */
		bool check_file(const std::string& name) {
			std::ifstream f(name.c_str());
			return f.good();
		}

		/**
		* \brief Read a shader source file
		* \param path Location of the shader file
		* \return Returns a string containing the shader source
		*/
		std::string readShaderFile(const char* const path)
		{
			std::ifstream inFile(path, std::ios::in);

			std::ostringstream source;
			while (inFile.good()) {
				int c = inFile.get();
				if (!inFile.eof()) source << (char)c;
			}
			inFile.close();

			return source.str();
		}
	}

	ImageViewer::ImageViewer(Rasterizer& rasterizer)
		: m_image(rasterizer.accessImage()), rasterizer(rasterizer), m_needs_update(true), m_auto_update(false)
	{ }

	void ImageViewer::run()
	{
		std::cout << "OpenGL ImageViewer"<<std::endl;

		// Initialize GLFW
		if (!glfwInit())
		{
			std::cout << "Error: Couldn't initialize glfw.";
		}

		this->m_active_window = glfwCreateWindow(this->m_image.get_width(), this->m_image.get_height(), "Rasterizer", NULL, NULL);

		if (!this->m_active_window)
		{
			std::cout << "Error: Couldn't open glfw window";

			glfwTerminate();
		}

		glfwMakeContextCurrent(this->m_active_window);

		// Register callback functions
		glfwSetWindowUserPointer(this->m_active_window, this);

		auto windowSizeCallbackStub = [](GLFWwindow* w, int width, int height)
		{
			static_cast<ImageViewer*>(glfwGetWindowUserPointer(w))->windowSizeCallback(w, width, height);
		};

		auto windowCloseCallbackStub = [](GLFWwindow* w)
		{
			static_cast<ImageViewer*>(glfwGetWindowUserPointer(w))->windowCloseCallback(w);
		};

		glfwSetWindowSizeCallback(this->m_active_window, windowSizeCallbackStub);
		glfwSetWindowCloseCallback(this->m_active_window, windowCloseCallbackStub);

		// Set install_callbacks to false cause I call them myself
		ImGui_ImplGlfwGL3_Init(this->m_active_window, true);

		if (!gladLoadGL()) {
			std::cout << "Something went wrong!\n";
			exit(-1);
		}
		
		// Load display texture shader program
		std::string display_shader_frag_src;
		std::string display_shader_vert_src;
		if(check_file("../../Aufgaben/shader/display_texture_v.glsl") && check_file("../../Aufgaben/shader/display_texture_f.glsl"))
		{
			display_shader_vert_src = readShaderFile("../../Aufgaben/shader/display_texture_v.glsl");
			display_shader_frag_src = readShaderFile("../../Aufgaben/shader/display_texture_f.glsl");
		}
		else if (check_file("shader/display_texture_v.glsl") && check_file("shader/display_texture_f.glsl"))
		{
			display_shader_vert_src = readShaderFile("shader/display_texture_v.glsl");
			display_shader_frag_src = readShaderFile("shader/display_texture_f.glsl");
		}
		else
		{
			std::cerr << "Could not located display_texture_*.glsl shader source files." << std::endl;
			return;
		}

		this->m_display_prgm = std::make_unique<GLSLProgram>();
		bool prgm_error = false;
		prgm_error |= !this->m_display_prgm->compileShaderFromString(&display_shader_vert_src, GL_VERTEX_SHADER);
		prgm_error |= !this->m_display_prgm->compileShaderFromString(&display_shader_frag_src, GL_FRAGMENT_SHADER);
		prgm_error |= !this->m_display_prgm->link();
		if (prgm_error)
		{
			std::cout << "Error during shader program creation of 'display_texture_*.glsl':" << std::endl;
			std::cout << this->m_display_prgm->getLog();
		}

		while (!glfwWindowShouldClose(this->m_active_window))
		{
			TextureLayout img_layout(GL_RGBA32F, this->m_image.get_width(), this->m_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
			img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
			img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER, GL_LINEAR });
			this->m_texture = std::make_unique<Texture2D>(img_layout, const_cast<std::array<float, 4>*>(this->m_image.data()));

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			int width, height;
			glfwGetFramebufferSize(this->m_active_window, &width, &height);
			glViewport(0, 0, width, height);

			this->m_display_prgm->use();

			glActiveTexture(GL_TEXTURE0);
			this->m_texture->bindTexture();
			glUniform1i(this->m_display_prgm->getUniformLocation("display_tx2D"), 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			ImGui_ImplGlfwGL3_NewFrame();
			drawUI();
			ImGui::Render();

			auto gl_err = glGetError();
			if (gl_err != GL_NO_ERROR)
				std::cerr << "GL error at end of frame: "<< gl_err << std::endl;

			glfwSwapBuffers(this->m_active_window);
			glfwPollEvents();
		}

		// Clean up GPU resources while context is still alive
		this->m_display_prgm.reset();
	}

	void ImageViewer::drawUI()
	{
		ImGuiWindowFlags window_flags = 0;
		bool p_open = true;
		if (!ImGui::Begin("Image Viewer", &p_open, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// Scene selection
		ImGui::Text("Scene");

		for (int i = 0; i < this->rasterizer.getNumScenes() - 1; ++i)
		{
			if (ImGui::RadioButton(this->rasterizer.getScenes()[i].getName().c_str(), this->rasterizer.accessActiveScene() == i))
			{
				if (this->rasterizer.accessActiveScene() != i)
				{
					this->m_needs_update = true;
				}

				this->rasterizer.accessActiveScene() = i;
			}

			ImGui::SameLine();
		}

		if (ImGui::RadioButton(this->rasterizer.getScenes()[this->rasterizer.getNumScenes() - 1].getName().c_str(),
			this->rasterizer.accessActiveScene() == this->rasterizer.getNumScenes() - 1))
		{
			if (this->rasterizer.accessActiveScene() != this->rasterizer.getNumScenes() - 1)
			{
				this->m_needs_update = true;
			}

			this->rasterizer.accessActiveScene() = this->rasterizer.getNumScenes() - 1;
		}

		ImGui::Separator();
		
		// Camera settings
		ImGui::Text("Camera");

		ImGui::DragFloat("Field of view", &this->rasterizer.accessCamera().accessFov(), 5.0f, 30.0f, 90.0f);
		ImGui::DragFloat("Near plane", &this->rasterizer.accessCamera().accessNear(), 0.01f, 0.001f, 10.0f);
		ImGui::DragFloat("Far plane", &this->rasterizer.accessCamera().accessFar(), 10.0f, 10.0f, 100000.0f);

		if (ImGui::Button("Reset", ImVec2(300, 20)))
		{
			this->rasterizer.accessCamera() = defaultCamera();
			this->rasterizer.accessCamera().setAspect(static_cast<float>(this->m_image.get_width()) / static_cast<float>(this->m_image.get_height()));

			this->m_needs_update = true;
		}
		
		ImGui::Separator();

		// Rasterizer settings
		ImGui::Text("Rasterization");

		const char* items[] = { "POINTS", "WIREFRAME", "FILLED" };
		static int item = this->rasterizer.accessMode();
		ImGui::Combo("Mode", &item, items, IM_ARRAYSIZE(items));
		this->rasterizer.accessMode() = static_cast<Rasterizer::rasterization_mode>(item);

		ImGui::Separator();

		// Rotate option
		ImGui::Text("Rotation animation");

		ImGui::SameLine();
		if (ImGui::RadioButton("On", this->m_rotate))
		{
			this->m_rotate = true;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("Off", !this->m_rotate))
		{
			this->m_rotate = false;
		}

		float axis[3] = { this->rasterizer.accessRotationAxis()[0], this->rasterizer.accessRotationAxis()[1], this->rasterizer.accessRotationAxis()[2] };
		ImGui::DragFloat3("Rotation axis", axis, 0.01f, 0.0f, 1.0f);
		this->rasterizer.accessRotationAxis() = vec3(axis[0], axis[1], axis[2]);

		ImGui::DragFloat("Rotation speed", &this->rasterizer.accessRotationSpeed(), 1.0f, 1.0f, 100.0f);

		ImGui::Separator();

		// Light options
		auto lights = this->rasterizer.accessScene().getLights();

		if (lights.size() != 0)
		{
			ImGui::Text("Lights");

			for (std::size_t i = 0; i < lights.size(); ++i)
			{
				const std::string object_name = "Use " + lights[i]->getShapeName() + " #" + std::to_string(i);
				const std::string color_name = "Light color " + std::to_string(i);
				const std::string intensity_name = "Light intensity " + std::to_string(i);

				ImGui::Checkbox(object_name.c_str(), &lights[i]->accessVisibility());

				float color[3] = { lights[i]->accessColor()[0], lights[i]->accessColor()[1], lights[i]->accessColor()[2] };
				ImGui::ColorEdit3(color_name.c_str(), color);
				lights[i]->accessColor() = Color(color[0], color[1], color[2], 1.0f);

				ImGui::DragFloat(intensity_name.c_str(), &lights[i]->accessIntensity(), 0.1f, 0.0f, 100.0f);
			}

			ImGui::Separator();
		}

		// Object options
		auto objects = this->rasterizer.accessScene().getObjects();

		if (objects.size() != 0)
		{
			ImGui::Text("Objects");

			for (std::size_t i = 0; i < objects.size(); ++i)
			{
				const std::string object_name = "Show " + objects[i]->getShapeName() + " #" + std::to_string(i);
				const std::string color_name = "Object color " + std::to_string(i);

				ImGui::Checkbox(object_name.c_str(), &objects[i]->accessVisibility());

				if (dynamic_cast<cg::Container*>(objects[i].get()) == nullptr)
				{
					float color[3] = { objects[i]->accessColor()[0], objects[i]->accessColor()[1], objects[i]->accessColor()[2] };
					ImGui::ColorEdit3(color_name.c_str(), color);
					objects[i]->accessColor() = Color(color[0], color[1], color[2], 1.0f);
				}
			}

			ImGui::Separator();
		}

		// Draw button
		ImGui::Checkbox("Auto update", &this->m_auto_update);

		if (ImGui::Button("Update", ImVec2(300, 20)) || this->m_rotate || this->m_needs_update || this->m_auto_update)
		{
			this->rasterizer.draw(this->m_rotate);
			this->m_needs_update = this->m_rotate;
		}

		ImGui::End();
	}

	void ImageViewer::windowSizeCallback(GLFWwindow* window, int width, int height)
	{
		this->rasterizer.accessCamera().setAspect(static_cast<float>(width) / static_cast<float>(height));
		this->rasterizer.accessImage().resize(width, height);

		this->m_needs_update = true;
	}

	void ImageViewer::windowCloseCallback(GLFWwindow* window)
	{ }
}