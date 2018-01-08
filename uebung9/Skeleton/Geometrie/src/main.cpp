// OpenGL example application, that renders a bunch of trees via instancing.

#include "core.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "lodepng/lodepng.h"

#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

/// Combination of a vertex array object and an index buffer object.
struct RenderBatch {
    GLuint vao; // vertex array object
    GLuint ibo; // index buffer object
    int32_t index_count;
};

/// Layout of the vertices, used to store them in vertex buffer objects.
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

/// Layout of the per-frame data submitted to the graphics device.
struct PerFrame {
    glm::mat4 view; // world coords -> clip coords
    glm::mat4 proj;
};

/// Layout of the per-object data submitted to the graphics device.
struct PerObject {
    glm::mat4 model; // object coords -> world coords
};

RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices,
                              const std::vector<uint32_t> &indices);

GLuint CreateSpriteTexture(std::string const& filename);

void InitRenderer();
void InitScene();
void UpdateShaders();
int KeyAxisValue(GLFWwindow *window, int key1, int key2);
bool NeedSceneUpdate();
void UpdateScene();
void RenderFrame();
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

GLFWwindow *g_window;
int g_window_width = 960, g_window_height = 540;

GLuint g_shader_program_ground = 0; // shader program for rendering the ground
GLuint g_shader_program_trees = 0;  // shader program for rendering the trees
GLuint g_shader_program_tree_imposter = 0; // shader program for rendering the tree imposters

RenderBatch g_render_batch_ground;        // geometry of the ground
RenderBatch g_render_batch_tree;          // geometry of a tree
RenderBatch g_render_batch_tree_imposter; // geometry of a tree imposter

GLuint g_texture_name_tree_sprite; // texture for the tree imposters

GLuint g_ubo_per_frame; // uniform buffer containing per-frame data
GLuint g_ubo_trees;     // uniform buffer containing per-object data for all trees

PerFrame g_per_frame; // local copy of the per-frame uniform buffer

glm::vec3 g_cam_pos{-10, 5, 12};   // camera position
glm::vec3 g_cam_velocity{0, 0, 0}; // change of g_cam_pos per tick
float g_cam_yaw = -0.7f;           // camera left-right orientation in [-pi, pi]
float g_diff_cam_yaw = 0.f;        // change of g_cam_yaw per tick
float g_cam_pitch = -0.2f;         // camera up down orientation in [-1.5, 1.5]
float g_diff_cam_pitch = 0.f;      // change of g_cam_pitch per tick

constexpr int numTrees = 192; // number of trees to render

/// Entry point into the OpenGL example application.
int main() {
    try {
        // initialize glfw
        GlfwInstance::init();

        // initialize this application
        InitRenderer();
        InitScene();

        // render until the window should close
        while (!glfwWindowShouldClose(g_window)) {
            glfwPollEvents(); // handle events
            while (NeedSceneUpdate())
                UpdateScene();         // per-tick updates
            RenderFrame();             // render image
            glfwSwapBuffers(g_window); // present image
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\nPress enter.\n";
        getchar();
        return -1;
    }
}

/// Creates the window and sets persistent render settings and compiles shaders.
/// @throw std::runtime_error If an error occurs during intialization.
void InitRenderer() {
    // create window
    g_window = glfwCreateWindow(g_window_width, g_window_height, // initial resolution
                                "Geometrie",                     // window title
                                nullptr, nullptr);
    if (!g_window)
        throw std::runtime_error("Failed to create window.");

    // use the window that was just created
    glfwMakeContextCurrent(g_window);

    // get window resolution
    glfwGetFramebufferSize(g_window, &g_window_width, &g_window_height);

    // set callbacks for when the resolution changes or a key is pressed
    glfwSetFramebufferSizeCallback(g_window, &FramebufferSizeCallback);
    glfwSetKeyCallback(g_window, &KeyCallback);

    // enable vsync
    glfwSwapInterval(1);

    // load OpenGL (return value of 0 indicates error)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Failed to load OpenGL context.");
    std::cout << "OpenGL " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

    // enable depth buffering
    glEnable(GL_DEPTH_TEST);

	// enable simple blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create shader programs
    UpdateShaders();

    // create per-frame uniform buffer (will be filled later, once per frame)
    glGenBuffers(1, &g_ubo_per_frame);
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_frame);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrame), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_ubo_per_frame); // binding 0
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/// Creates geometry and uniform buffers.
void InitScene() {
    // create ground geometry: each vertex consists of a position and a color
    std::vector<Vertex> ground_vertices{{{-20.5f, 0.0f, -20.5f}, {0.5f, 0.5f, 0.5f}},
                                        {{-20.5f, 0.0f, 20.5f}, {0.5f, 0.5f, 0.5f}},
                                        {{20.5f, 0.0f, 20.5f}, {0.5f, 0.5f, 0.5f}},
                                        {{20.5f, 0.0f, -20.5f}, {0.5f, 0.5f, 0.5f}}};
    std::vector<uint32_t> ground_indices{0, 1, 2, 0, 2, 3};
    g_render_batch_ground = CreateRenderBatch(ground_vertices, ground_indices);

    /*
     * TODO (zu Teilaufgabe 1.2): Erstellen Sie an dieser Stelle analog die Geometrie eines
     * zweidimensionalen Baumes und initialisieren Sie damit g_render_batch_tree.
     */
	std::vector<Vertex> tree_vertices{ { { 0.0f, 0.0f, 0.0f },{ 0.54509803921f, 0.27058823529f, 0.07450980392f } },
									   { { 1.0f, 0.0f, 0.0f },{ 0.54509803921f, 0.27058823529f, 0.07450980392f } },
									   { { 1.0f, 1.0f, 0.0f },{ 0.54509803921f, 0.27058823529f, 0.07450980392f } },
									   { { 0.0f, 1.0f, 0.0f },{ 0.54509803921f, 0.27058823529f, 0.07450980392f } },
									   { { -1.0f, 1.0f, 0.0f },{ 0.0f, 0.5f, 0.0f } },
									   { { 2.0f, 1.0f, 0.0f },{ 0.0f, 0.5f, 0.0f } },
									   { { 0.5f, 4.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } } //mega ultra super fancy spitze
									 };
	std::vector<uint32_t> tree_indices{ 0, 1, 2, 0, 2, 3, 4, 5, 6 };
	g_render_batch_tree = CreateRenderBatch(tree_vertices, tree_indices);

	 /*
	 * TODO (zu Teilaufgabe 2.1): Erstellen Sie an dieser Stelle analog die Geometrie eines
	 * zweidimensionalen Quads und initialisieren Sie damit g_render_batch_tree_imposter.
	 *
	 * Das Quad soll im Bereich [-1.5 , 1.5] x [0.0 , 3.0] liegen und Texturkoordinaten
	 * im Bereich [0,1] x [1,0] haben.
	 * Verwenden sie die ersten beiden Eintraege des Color-Attributes der Vertices um die
	 * Texturkoordinaten zu speichern (setzen sie den dritten Eintrag einfach auf 0).
	 */

	 // create tree sprite texture
	g_texture_name_tree_sprite = CreateSpriteTexture("resources/pine_tree_sprite.png");

    // create transformations used for instanced rendering of the trees
    std::array<glm::mat4, numTrees> model_trees;
    std::default_random_engine generator;
    std::uniform_int_distribution<> dist{-20, 20};
    for (auto &model : model_trees) {
        int x = dist(generator);
        int z = dist(generator);
        model = glm::translate(glm::mat4{}, glm::vec3{x, 0, z});
    }

    // create & fill uniform buffer holding the transformations
    glGenBuffers(1, &g_ubo_trees);
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_trees);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PerObject) * model_trees.size(), model_trees.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_ubo_trees);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/// @param vertices A vertex buffer.
/// @param indices An index buffer.
/// @return RenderBatch object created using the given buffers.
RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices,
                              const std::vector<uint32_t> &indices) {
    RenderBatch job;
    job.index_count = static_cast<int32_t>(indices.size());

    // create, bind & fill index buffer
    glGenBuffers(1, &job.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, job.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(uint32_t) * indices.size(), // data size
                 indices.data(),                    // data pointer
                 GL_STATIC_DRAW);                   // usage
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // create & bind vertex array object
    glGenVertexArrays(1, &job.vao);
    glBindVertexArray(job.vao);

    // create, bind & fill vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * vertices.size(), // data size
                 vertices.data(),                  // data pointer
                 GL_STATIC_DRAW);                  // usage

    // specify & enable position attribute (the data is taken from the currently bound vbo)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,           // index of the attribute
        3, GL_FLOAT, // a position consists of 3 floats
        GL_FALSE,
        sizeof(Vertex), // stride between consecutive vertices in the buffer
        reinterpret_cast<const void *>(offsetof(Vertex, position))); // offset of this attribute

    // specify & enable color attribute (the data is taken from the currently bound vbo)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,           // index of the attribute
        3, GL_FLOAT, // a color consists of 3 floats
        GL_FALSE,
        sizeof(Vertex), // stride between consecutive colors in the buffer
        reinterpret_cast<const void *>(offsetof(Vertex, color))); // offset of this attribute

    // unbind vertex array object & vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return job;
}


GLuint CreateSpriteTexture(std::string const& filename)
{
	// Load file and decode image.
	unsigned int width, height;
	std::vector<unsigned char> image_data;
	unsigned int error = lodepng::decode(image_data, width, height, filename);

	GLint level = 0;
	GLint internal_format = GL_RGBA8;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;
	GLint border = 0;

	GLuint texture_name = 0;

	/*
	* TODO (zu Teilaufgabe 2.3): Erstellen ein OpenGL Texture Object.
	* ANFANG
	*/

	// generate a new texture object using glGenTextures and store its name in texture_name

	// bind the created texture as a 2D texture

	// use glTexParameteri to set the texture wrap mode to GL_MIRRORED_REPEAT,
	// the minification filter to GL_LINEAR_MIPMAP_LINEAR and the maxification filter to GL_LINEAR
	// hint: you have to do four calls to the function

	// use the (oldschool) function glTexImage2D to buffer the actual texture data using the information and data supplied above

	// generate mipmaps for the texture

	/*
	* TODO (zu Teilaufgabe 2.3)
	* ENDE
	*/

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture_name;
}

/// Loads the shader programs or reloads them.
void UpdateShaders() {
	glDeleteProgram(g_shader_program_ground);
	glDeleteProgram(g_shader_program_trees);

	try {
		// compile vertex & fragment shader
		GLuint vertex_shader_ground = glCreateShader(GL_VERTEX_SHADER);
		CompileShader(vertex_shader_ground, "shaders/vert_ground.glsl");
		GLuint vertex_shader_trees = glCreateShader(GL_VERTEX_SHADER);
		CompileShader(vertex_shader_trees, "shaders/vert_trees.glsl");
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		CompileShader(fragment_shader, "shaders/frag.glsl");

		GLuint vertex_shdr_tree_sprite = glCreateShader(GL_VERTEX_SHADER);
		CompileShader(vertex_shdr_tree_sprite, "shaders/vert_tree_imposter.glsl");
		GLuint fragment_shdr_tree_sprite = glCreateShader(GL_FRAGMENT_SHADER);
		CompileShader(fragment_shdr_tree_sprite, "shaders/frag_tree_imposter.glsl");

		// link shaders
		g_shader_program_ground = glCreateProgram();
		LinkProgram(g_shader_program_ground, { vertex_shader_ground, fragment_shader });
		g_shader_program_trees = glCreateProgram();
		LinkProgram(g_shader_program_trees, { vertex_shader_trees, fragment_shader });
		g_shader_program_tree_imposter = glCreateProgram();
		LinkProgram(g_shader_program_tree_imposter, { vertex_shdr_tree_sprite, fragment_shdr_tree_sprite });

		// clean shaders up
		glDetachShader(g_shader_program_trees, fragment_shader);
		glDetachShader(g_shader_program_trees, vertex_shader_trees);
		glDetachShader(g_shader_program_ground, fragment_shader);
		glDetachShader(g_shader_program_ground, vertex_shader_ground);
		glDetachShader(g_shader_program_tree_imposter, vertex_shdr_tree_sprite);
		glDetachShader(g_shader_program_tree_imposter, fragment_shdr_tree_sprite);
		glDeleteShader(fragment_shader);
		glDeleteShader(vertex_shader_trees);
		glDeleteShader(vertex_shader_ground);
		glDeleteShader(vertex_shdr_tree_sprite);
		glDeleteShader(fragment_shdr_tree_sprite);
	}
	catch (const std::runtime_error &e) {
		std::cerr << e.what() << "\nPress 'R' to reload the shaders.\n";
	}
}

/// Helper function for controlling an axis with 2 keys.
/// @param window The window where the keys should be pressed.
/// @param key1 The first key.
/// @param key2 The second key.
/// @return -1 if only the first key is pressed, 1 if only the second key is pressed and 0 if none
/// of the keys or both keys are pressed.
int KeyAxisValue(GLFWwindow *window, int key1, int key2) {
    bool key1_pressed = glfwGetKey(g_window, key1) == GLFW_PRESS;
    bool key2_pressed = glfwGetKey(g_window, key2) == GLFW_PRESS;
    return key2_pressed - key1_pressed;
}

/// Synchronizes scene updates to a frequenzy of 100 Hz.
/// This function assumes that the scene is updated each time it returns true.
/// @return true iff the scene should be updated now.
bool NeedSceneUpdate() {
    static auto nextUpdate = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    if (nextUpdate > now)
        return false;
    nextUpdate += std::chrono::milliseconds{10};
    return true;
}

/// Applies per-frame updates of the scene.
void UpdateScene() {
    // update camera yaw and pitch
    int rot_horz_target = KeyAxisValue(g_window, GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
    int rot_vert_target = KeyAxisValue(g_window, GLFW_KEY_DOWN, GLFW_KEY_UP);
    g_diff_cam_yaw = lerp(0.1f, g_diff_cam_yaw, static_cast<float>(rot_horz_target));
    g_diff_cam_pitch = lerp(0.1f, g_diff_cam_pitch, static_cast<float>(rot_vert_target));
    g_cam_yaw += 0.03f * g_diff_cam_yaw;
    g_cam_yaw = std::remainder(g_cam_yaw, 2 * glm::pi<float>());
    g_cam_pitch += 0.03f * g_diff_cam_pitch;
    g_cam_pitch = glm::clamp(g_cam_pitch, -1.5f, 1.5f);

    // calculate camera direction
    glm::vec3 cam_dir{0, 0, -1};
    cam_dir = glm::mat3(glm::rotate(glm::mat4(), g_cam_pitch, glm::vec3(1, 0, 0))) * cam_dir;
    cam_dir = glm::mat3(glm::rotate(glm::mat4(), g_cam_yaw, glm::vec3(0, 1, 0))) * cam_dir;

    // calculate the rotation matrix used to orient the target velocity along the camera
    glm::vec2 cam_dir_2d{cam_dir.x, cam_dir.z}; // only consider the xz-plane
    cam_dir_2d = glm::normalize(cam_dir_2d);
    glm::mat3 rot{-cam_dir_2d.y, 0, cam_dir_2d.x, 0, 1, 0, -cam_dir_2d.x, 0, -cam_dir_2d.y};

    // update camera velocity and position
    glm::vec3 target_velocity =
        rot * glm::vec3{KeyAxisValue(g_window, GLFW_KEY_A, GLFW_KEY_D),
                        KeyAxisValue(g_window, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE),
                        KeyAxisValue(g_window, GLFW_KEY_W, GLFW_KEY_S)};
    g_cam_velocity = lerp(0.08f, g_cam_velocity, target_velocity);
    g_cam_pos += 0.05f * g_cam_velocity;

    /*
     * TODO (zu Teilaufgabe 1.1): Schreiben Sie die View-Matrix nach g_per_frame.view und die
     * Projection-Matrix nach g_per_frame.proj.
     */
	g_per_frame.view = glm::lookAt(g_cam_pos, cam_dir, glm::vec3(0, 1, 0));
	g_per_frame.proj = glm::perspective(glm::radians(90.0f), float(g_window_width) / g_window_height, 0.1f, 100.0f);
}

/// Renders a frame.
void RenderFrame() {
    // update per-frame uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_frame);
    void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(data, &g_per_frame, sizeof(g_per_frame));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // setup rendering
    glViewport(0, 0, g_window_width, g_window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the ground
    glUseProgram(g_shader_program_ground);
    glBindVertexArray(g_render_batch_ground.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_batch_ground.ibo);
    glDrawElements(GL_TRIANGLES, g_render_batch_ground.index_count, GL_UNSIGNED_INT, nullptr);

    // render the trees
    glUseProgram(g_shader_program_trees);
    glBindVertexArray(g_render_batch_tree.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_batch_tree.ibo);
    glDrawElementsInstanced(GL_TRIANGLES, g_render_batch_tree.index_count, GL_UNSIGNED_INT, nullptr,
                            numTrees);

	// render the tree sprites
	glUseProgram(g_shader_program_tree_imposter);
	glActiveTexture(GL_TEXTURE0); // activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, g_texture_name_tree_sprite); // bind texture object to active texture unit
	glUniform1i(glGetUniformLocation(g_shader_program_tree_imposter, "tree_sprite_tx2D"), 0); // set shader uniform to match texture unit
	glBindVertexArray(g_render_batch_tree_imposter.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_batch_tree_imposter.ibo);
	glDrawElementsInstanced(GL_TRIANGLES, g_render_batch_tree_imposter.index_count, GL_UNSIGNED_INT, nullptr,
		numTrees);

    // unbind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/// Updates the resolution. This is called by GLFW when the resolution changes.
/// @param window The window that changed resolution.
/// @param width New width.
/// @param height New height.
void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    g_window_width = width;
    g_window_height = height;
}

/// Reloads the shaders if 'R' is pressed. This is called by GLFW when a key is pressed.
/// @param window The window in which a key was pressed.
/// @param key GLFW key code of the pressed key.
/// @param scancode platform-specific key code.
/// @param action One of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
/// @param mods Modifier bits.
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        UpdateShaders();
}
