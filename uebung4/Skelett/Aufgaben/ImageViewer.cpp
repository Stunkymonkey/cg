#include "ImageViewer.h"

#include <iostream>
#include <sstream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include <GLFW/glfw3.h>

namespace cg
{

	namespace
	{
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

	ImageViewer::ImageViewer()
		: m_original_image(1,1),
		m_compute_mode(CPU),
		m_active_mode(CPU_GAUSSIAN_2D),
		m_active_border_policy(filter::BorderPolicy::CLAMP_TO_EDGE),
		m_extents({1,1}),
		m_sigma(3.0f)
	{
		try
		{
			// Read RGB source image file
			if (check_file("../../../Bilder/lena.ppm") )
			{
				m_original_image = image_io::load_padded_rgba_image("../../../Bilder/lena.ppm");
				m_image_path = "../../../Bilder/lena.ppm";
			}
			else if (check_file("../../Bilder/lena.ppm"))
			{
				m_original_image = image_io::load_padded_rgba_image("../../Bilder/lena.ppm");
				m_image_path = "../../Bilder/lena.ppm";
			}
			else if (check_file("../Bilder/lena.ppm"))
			{
				m_original_image = image_io::load_padded_rgba_image("../Bilder/lena.ppm");
				m_image_path = "../Bilder/lena.ppm";
			}
			else
			{
				std::cerr << "Could not located default image file. Please specify path manually." << std::endl;
				m_image_path = "*.ppm";
			}

		}
		catch (const std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Unknown error" << std::endl;
		}
	}

	void ImageViewer::run()
	{
		std::cout << "OpenGL ImageViewer"<<std::endl;

		// Initialize GLFW
		if (!glfwInit())
		{
			std::cout << "Error: Couldn't initialize glfw.";
		}

		m_active_window = glfwCreateWindow(512, 512, "Image Viewer", NULL, NULL);

		if (!m_active_window)
		{
			std::cout << "Error: Couldn't open glfw window";

			glfwTerminate();
		}

		glfwMakeContextCurrent(m_active_window);

		// Register callback functions
		glfwSetWindowSizeCallback(m_active_window, windowSizeCallback);
		glfwSetWindowCloseCallback(m_active_window, windowCloseCallback);

		glfwSetWindowUserPointer(m_active_window, this);

		// Set install_callbacks to false cause I call them myself
		ImGui_ImplGlfwGL3_Init(m_active_window, true);

		if (!gladLoadGL()) {
			std::cout << "Something went wrong!\n";
			exit(-1);
		}

		// gladLoadGLLoader(&glutGetProcAddress);
		std::cout << "Using OpenGL Context Version " << GLVersion.major <<"."<<GLVersion.minor << std::endl;
		if (GLVersion.major < 4) {
			std::cout<<"Your system doesn't support OpenGL >= 4! Deactivating GPGPU-Compute Option. \n";
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

		m_display_prgm = std::make_unique<GLSLProgram>();
		bool prgm_error = false;
		prgm_error |= !m_display_prgm->compileShaderFromString(&display_shader_vert_src, GL_VERTEX_SHADER);
		prgm_error |= !m_display_prgm->compileShaderFromString(&display_shader_frag_src, GL_FRAGMENT_SHADER);
		prgm_error |= !m_display_prgm->link();
		if (prgm_error)
		{
			std::cout << "Error during shader program creation of 'display_texture_*.glsl':" << std::endl;
			std::cout << m_display_prgm->getLog();
		}

		std::string seperated_gaussian_compute_src;
		if (check_file("../../Aufgaben/shader/seperated_gaussian_c.glsl"))
		{
			seperated_gaussian_compute_src = readShaderFile("../../Aufgaben/shader/seperated_gaussian_c.glsl");
		}
		else if (check_file("shader/seperated_gaussian_c.glsl"))
		{
			seperated_gaussian_compute_src = readShaderFile("shader/seperated_gaussian_c.glsl");
		}
		else
		{
			std::cerr << "Could not located seperated_gausian_c.glsl shader source files." << std::endl;
			return;
		}

		if (GLVersion.major >= 4 && GLVersion.minor >= 3)
		{
			m_filter_prgm = std::make_unique<GLSLProgram>();
			prgm_error = false;
			prgm_error |= !m_filter_prgm->compileShaderFromString(&seperated_gaussian_compute_src, GL_COMPUTE_SHADER);
			prgm_error |= !m_filter_prgm->link();
			if (prgm_error)
			{
				std::cout << "Error during shader program creation of 'seperated_gausian_c.glsl':" << std::endl;
				std::cout << m_filter_prgm->getLog();
			}
		}

		// Intially, store original image in display texture
		TextureLayout img_layout(GL_RGBA32F, m_original_image.get_width(), m_original_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER, GL_LINEAR });
		std::get<0>(m_textures) = std::make_unique<Texture2D>(img_layout, m_original_image.data());
		std::get<1>(m_textures) = std::make_unique<Texture2D>(img_layout, nullptr);
		std::get<2>(m_textures) = std::make_unique<Texture2D>(img_layout, m_original_image.data());

		while (!glfwWindowShouldClose(m_active_window))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			int width, height;
			glfwGetFramebufferSize(m_active_window, &width, &height);
			glViewport(0, 0, width, height);

			m_display_prgm->use();

			glActiveTexture(GL_TEXTURE0);
			std::get<2>(m_textures)->bindTexture();
			glUniform1i(m_display_prgm->getUniformLocation("display_tx2D"), 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			ImGui_ImplGlfwGL3_NewFrame();
			drawUI();
			ImGui::Render();

			auto gl_err = glGetError();
			if (gl_err != GL_NO_ERROR)
				std::cerr << "GL error at end of frame: "<< gl_err << std::endl;

			glfwSwapBuffers(m_active_window);
			glfwPollEvents();
		}

		// Clean up GPU resources while context is still alive
		m_display_prgm.reset();
		m_filter_prgm.reset();
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

		ImGui::Text("Image");
		ImGui::SameLine();

		static std::vector<char> img_path(m_image_path.c_str(), m_image_path.c_str() + m_image_path.length() + 1);
		img_path.resize(img_path.size() + 50);
		if (ImGui::InputText("", img_path.data(), 100,ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (check_file(std::string(img_path.data())))
			{
				m_original_image = image_io::load_padded_rgba_image(std::string(img_path.data()));

				TextureLayout img_layout(GL_RGBA32F, m_original_image.get_width(), m_original_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
				img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
				img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
				img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
				img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
				std::get<0>(m_textures)->reload(img_layout, m_original_image.data());
				std::get<1>(m_textures)->reload(img_layout, nullptr);
				std::get<2>(m_textures)->reload(img_layout, m_original_image.data());
			}
		}

		ImGui::Separator();

		ImGui::Text("Using");
		ImGui::SameLine();
		if (ImGui::RadioButton("CPU", m_compute_mode == CPU))
		{
			m_compute_mode = CPU;
		}
		
		if(m_filter_prgm)
		{
			ImGui::SameLine();
			if (ImGui::RadioButton("GPU", !(m_compute_mode == CPU)))
			{
				m_compute_mode = GPU;
			}	
		}

		ImGui::Separator();

		if (m_compute_mode == CPU)
		{
			const char* items[] = { "GAUSSIAN", "SEPERATED GAUSSIAN", "EDGE_DETECTION" };
			static int item = 0;
			ImGui::Combo("Filter", &item, items, IM_ARRAYSIZE(items));
			
			switch (item)
			{
			case 0:
				m_active_mode = CPU_GAUSSIAN_2D;
				break;
			case 1:
				m_active_mode = CPU_SEPERATED_GAUSSIAN;
				break;
			case 2:
				m_active_mode = CPU_EDGE_DETECTION;
				break;
			default:
				m_active_mode = CPU_GAUSSIAN_2D;
				break;
			}
		}
		else
		{
			const char* items[] = { "SEPERATED GAUSSIAN" };
			static int item = 0;
			ImGui::Combo("Filter", &item, items, IM_ARRAYSIZE(items));

			m_active_mode = GPU_SEPERATED_GAUSSIAN;
		}

		if (m_active_mode == CPU_GAUSSIAN_2D ||
			m_active_mode == CPU_SEPERATED_GAUSSIAN ||
			m_active_mode == GPU_SEPERATED_GAUSSIAN)
		{
			ImGui::DragFloat("Gaussian sigma", &m_sigma, 0.01f, 0.001f, 10.0f);

			ImGui::InputInt2("Filter extents", &m_extents.first);
		}

		if (m_compute_mode == CPU)
		{
			const char* items[] = { "CLAMP_TO_EDGE", "MIRROR", "REPEAT" };
			static int item = m_active_border_policy;
			ImGui::Combo("Border policy", &item, items, IM_ARRAYSIZE(items));
			m_active_border_policy = static_cast<filter::BorderPolicy>(item);
		}

		ImGui::Separator();

		if (ImGui::Button("Update", ImVec2(100, 20)))
		{
			updateDisplayImage();
		}

		ImGui::End();
	}

	void ImageViewer::updateDisplayImage()
	{
		switch (m_active_mode)
		{
		case cg::ImageViewer::CPU_GAUSSIAN_2D:
			applyCPUGaussian2D();
			break;
		case cg::ImageViewer::CPU_SEPERATED_GAUSSIAN:
			applyCPUSeperatedGaussian();
			break;
		case cg::ImageViewer::GPU_SEPERATED_GAUSSIAN:
			applyGPUSeperatedGaussian();
			break;
		case cg::ImageViewer::CPU_EDGE_DETECTION:
			applyCPUEdgeDetection();
			break;
		default:
			break;
		}
	}

	void ImageViewer::applyCPUGaussian2D()
	{
		///////
		// TODO
		// Generate a 2D gaussian filter kernel.
		// Use it to filter the original image and store the result in img.
		// Input value for the kernel can be accessed via m_extents and m_sigma.
		image<color_space_t::RGBA> img = m_original_image;
		filter::Kernel k = filter::build2DGaussianKernel();
		// iterate over every pixel
		// problem is, we were not able to compile because of intel
		for(int i; i = img.get_height(); i++) {
			for (int j; j = img.get_width(); j++) {
				//iterate over kernel
				for (int l = k.getHorizontalRange()[0]; l <= k.getHorizontalRange()[1]; l++) {
					for (int m = k.getVerticalRange()[0]; m <= k.getVerticalRange()[1]; m++) {
						img[j, i] +=  k.getDataIndex(m, l) * m_original_image[i+l, j+m];
					}
				}
			}
		}

		TextureLayout img_layout(GL_RGBA32F, m_original_image.get_width(), m_original_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		std::get<2>(m_textures)->reload(img_layout, img.data());
	}

	void ImageViewer::applyCPUSeperatedGaussian()
	{
		///////
		// TODO
		// Generate seperated gaussian filter kernels.
		// Use them to filter the original image and store the result in img.
		// Input value for the kernels can be accessed via m_extents and m_sigma.
		image<color_space_t::RGBA> img = m_original_image;
		filter::Kernel k1 = filter::build1DHorizontalGaussianKernel();
		filter::Kernel k2 = filter::build1DVerticalGaussianKernel();
		// iterate over every pixel
		// problem is, we were not able to compile because of intel
		for(int i; i = img.get_height(); i++) {
			for (int j; j = img.get_width(); j++) {
				//iterate over kernel
				for (int l = k.getHorizontalRange()[0]; l <= k.getHorizontalRange()[1]; l++) {
					img[j, i] +=  k.getDataIndex(l) * m_original_image[i+l, j];
				}
				for (int m = k.getVerticalRange()[0]; m <= k.getVerticalRange()[1]; m++) {
					img[j, i] +=  k.getDataIndex(m) * m_original_image[i, j+m];
				}
			}
		}

		TextureLayout img_layout(GL_RGBA32F, m_original_image.get_width(), m_original_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		std::get<2>(m_textures)->reload(img_layout, img.data());
	}

	void ImageViewer::applyGPUSeperatedGaussian()
	{
		m_filter_prgm->use();

		std::array<int,2> size = {
			static_cast<int>(m_original_image.get_width()), 
			static_cast<int>(m_original_image.get_height())
		};
		glUniform2iv(m_filter_prgm->getUniformLocation("img_size"), 1, size.data());
		glUniform1i(m_filter_prgm->getUniformLocation("kernel_extents"), std::get<0>(m_extents));

		auto kh = filter::build1DHorizontalGaussianKernel(std::get<0>(m_extents), m_sigma);
		TextureLayout kernel_layout(GL_R32F, std::get<0>(m_extents) *2+1, 1, 1, GL_RED, GL_FLOAT, 1);
		Texture2D kernel(kernel_layout, kh.data());
		kernel.bindImage(2, GL_READ_ONLY);
		glUniform1i(m_filter_prgm->getUniformLocation("kernel_tx2D"), 2);

		// Horizontal filter pass
		std::get<0>(m_textures)->bindImage(0, GL_READ_ONLY);
		glUniform1i(m_filter_prgm->getUniformLocation("src_tx2D"), 0);
		std::get<1>(m_textures)->bindImage(1, GL_WRITE_ONLY);
		glUniform1i(m_filter_prgm->getUniformLocation("tgt_tx2D"), 1);

		std::array<int, 2> offset = { 1,0 };
		glUniform2iv(m_filter_prgm->getUniformLocation("pixel_offset"), 1, offset.data());

		m_filter_prgm->dispatchCompute(m_original_image.get_width() / 4, m_original_image.get_height() / 4, 1);

		glMemoryBarrier(GL_COMPUTE_SHADER_BIT);

		// Vertical filter pass
		std::get<2>(m_textures)->bindImage(0, GL_WRITE_ONLY);
		glUniform1i(m_filter_prgm->getUniformLocation("tgt_tx2D"), 0);
		std::get<1>(m_textures)->bindImage(1, GL_READ_ONLY);
		glUniform1i(m_filter_prgm->getUniformLocation("src_tx2D"), 1);

		offset = { 0,1 };
		glUniform2iv(m_filter_prgm->getUniformLocation("pixel_offset"), 1, offset.data());

		m_filter_prgm->dispatchCompute(m_original_image.get_width() / 4, m_original_image.get_height() / 4, 1);
	}

	void ImageViewer::applyCPUEdgeDetection()
	{
		///////
		// TODO
		// Generate an edge detection filter kernel.
		// Use it to filter the original image and store the result in img.
		image<color_space_t::RGBA> img = m_original_image;
		filter::Kernel k = filter::buildEdgeDetectionKernel();
		// iterate over every pixel
		// problem is, we were not able to compile because of intel
		for(int i; i = img.get_height(); i++) {
			for (int j; j = img.get_width(); j++) {
				//iterate over kernel
				for (int l = k.getHorizontalRange()[0]; l <= k.getHorizontalRange()[1]; l++) {
					for (int m = k.getVerticalRange()[0]; m <= k.getVerticalRange()[1]; m++) {
						img[j, i] += k.getDataIndex(m, l) * m_original_image[i+l, j+m];
					}
				}
			}
		}


		TextureLayout img_layout(GL_RGBA32F, m_original_image.get_width(), m_original_image.get_height(), 1, GL_RGBA, GL_FLOAT, 1);
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
		img_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		std::get<2>(m_textures)->reload(img_layout, img.data());
	}

	void ImageViewer::windowSizeCallback(GLFWwindow* window, int width, int height)
	{

	}

	void ImageViewer::windowCloseCallback(GLFWwindow* window)
	{

	}
}