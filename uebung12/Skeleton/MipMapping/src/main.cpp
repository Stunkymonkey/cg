// OpenGL example application, that demonstrates mip mapping.

#include <core.hpp>
#include <simplex_noise.hpp>
#include <texture.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <tuple>

void InitRenderer();
void UpdateShaders();
void UpdateGroundTexture(GLint min_filter);
void UpdateGroundMaterialTextures();
void UpdateFramebuffers();
void InitScene();
RenderBatch CreateElephant();
RenderBatch CreateGround();
float GetGroundHeight(glm::ivec2 coords);
glm::vec3 GetGroundNormal(glm::ivec2 coords);
RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices,
                              const std::vector<uint32_t> &indices, bool use_texture,
                              const glm::mat4 &model = glm::mat4{});
bool NeedSceneUpdate();
void UpdateScene();
void RenderFrame();
void RenderObject(const RenderBatch &batch);
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

GLFWwindow *g_window;
int g_window_width = 960, g_window_height = 540;

double g_t0, g_t1 = 0.0;

GLuint g_shader_program_shadowmap = 0;			// for the 1st render pass where the shadow map is rendered
GLuint g_shader_program_final = 0;				// for the 2nd render pass where the geometry is rendered
GLuint g_shader_program_ground_material = 0;	// for rendering the material textured ground in the 2nd pass

const int g_shadowmap_width = 2048, g_shadowmap_height = 2048;
GLuint g_tex_shadowmap; // color buffer for the 1st render pass, for the shadow map's depth values
GLuint g_tex_shadowmap_depth;   // depth buffer for the 1st render pass
GLuint g_framebuffer_shadowmap; // framebuffer for the 1st render pass

std::vector<Texture> g_ground_textures; // raw data for the different mipmap levels of g_tex_ground
GLuint g_tex_ground = 0;                // texture for the ground, used in the 2nd render pass
bool g_show_material_texture = false;	// switch for displaying material textures

std::vector<Texture> g_material_textures;		 // raw data for the material texture maps
std::vector<GLuint> g_material_texture_names;	 // OpenGL texture names for material texture maps

GLuint g_ubo_per_frame;  // uniform buffer containing per-frame data
GLuint g_ubo_per_object; // uniform buffer containing per-object data

PerFrame g_per_frame;                // local copy of the per-frame uniform buffer
RenderBatch g_render_batch_elephant; // geometry of the elephant
RenderBatch g_render_batch_ground;   // geometry of the ground, generated from height map
const int g_ground_extent = 512;
const float g_ground_y_scale = 30.0f;

glm::vec3 g_cam_pos{-8, 5, 10};    // camera position
glm::vec3 g_cam_velocity{0, 0, 0}; // change of g_cam_pos per tick

float g_cam_yaw = -0.85f;     // camera left-right orientation in [-pi, pi]
float g_diff_cam_yaw = 0.f;   // change of g_cam_yaw per tick
float g_cam_pitch = -0.12f;   // camera up down orientation in [-1.5, 1.5]
float g_diff_cam_pitch = 0.f; // change of g_cam_pitch per tick

float g_light_yaw = 0.0f; // light horizontal orientation in [-pi, pi]

/// Entry point into the OpenGL example application.
int main()
{
    try
    {
        // initialize glfw
        GlfwInstance instance;

        // initialize this application
        InitRenderer();
        InitScene();

        // render until the window should close
        while (!glfwWindowShouldClose(g_window))
        {
            glfwPollEvents(); // handle events
            while (NeedSceneUpdate())
                UpdateScene();         // per-tick updates
            RenderFrame();             // render image
            glfwSwapBuffers(g_window); // present image
        }

        // clean up OpenGL objects
        glDeleteProgram(g_shader_program_shadowmap);
        glDeleteProgram(g_shader_program_final);
        glDeleteProgram(g_shader_program_ground_material);

        // should also clean up VBOs, IBOs, UBOs and textures

    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\nPress enter.\n";
        getchar();
        return -1;
    }
}

/// Creates the window and sets persistent render settings and compiles shaders.
/// @throw std::runtime_error If an ThrowError occurs during intialization.
void InitRenderer()
{
    // create window
    g_window = glfwCreateWindow(g_window_width, g_window_height, // initial resolution
                                "Mip Mapping",                   // window title
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

    // load OpenGL (return value of 0 indicates ThrowError)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Failed to load OpenGL context.");
    std::cout << "OpenGL " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

    // enable depth buffering
    glEnable(GL_DEPTH_TEST);

    // enable back-face culling
    glEnable(GL_CULL_FACE);

    // create shader programs
    UpdateShaders();

    // create the per-frame uniform buffer (will be filled later, once per frame)
    glGenBuffers(1, &g_ubo_per_frame);
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_frame);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrame), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_ubo_per_frame); // binding 0
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // create the per-object uniform buffer (will be filled later, once per rendering an object)
    glGenBuffers(1, &g_ubo_per_object);
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_object);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PerObject), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_ubo_per_object); // binding 1
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // create the intermediate framebuffer to render the shadowmap to
    glGenTextures(1, &g_tex_shadowmap);
    glGenTextures(1, &g_tex_shadowmap_depth);
    glGenFramebuffers(1, &g_framebuffer_shadowmap);

    UpdateFramebuffers();

    // create the texture containing the mip-map and use GL_NEAREST as minification filter
    std::vector<std::string> texture_paths{"textures/tex256.ppm", "textures/tex128.ppm",
                                            "textures/tex64.ppm",  "textures/tex32.ppm",
                                            "textures/tex16.ppm",  "textures/tex8.ppm"};
    for (auto& file_path : texture_paths)
    {
        g_ground_textures.emplace_back(file_path);
    }
    UpdateGroundTexture(GL_NEAREST);

    // load material textures and create texture objects
    std::vector<std::string> material_texture_paths{
        "textures/material_textures/sand_base_color.ppm", "textures/material_textures/drydirt_base_color.ppm",
        "textures/material_textures/grass_base_color.ppm", "textures/material_textures/rocksliced_base_color.ppm",
        "textures/material_textures/slatecliffrock_base_color.ppm", "textures/material_textures/slatecliffrock_base_color.ppm" };
    for (auto& file_path : material_texture_paths)
    {
        g_material_textures.emplace_back(file_path);
        g_material_texture_names.push_back(0);
    }
    UpdateGroundMaterialTextures();
}

/// Loads the shader programs or reloads them.
void UpdateShaders()
{
    std::cout << "Updating shaders." << std::endl;

    glDeleteProgram(g_shader_program_shadowmap);
    glDeleteProgram(g_shader_program_final);
    glDeleteProgram(g_shader_program_ground_material);
    try
    {
        g_shader_program_shadowmap =
            CreateShaderProgram("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");
        g_shader_program_final =
            CreateShaderProgram("shaders/final_vert.glsl", "shaders/final_frag.glsl");
        g_shader_program_ground_material =
            CreateShaderProgram("shaders/ground_material_vert.glsl", "shaders/ground_material_frag.glsl");
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << "\nPress 'R' to reload the shaders.\n";
    }
}

/// Updates the texture containing the mip-map.
void UpdateGroundTexture(GLint min_filter)
{
    // generate texture object (but only once)
    if(g_tex_ground == 0)
        glGenTextures(1, &g_tex_ground);

    glActiveTexture(GL_TEXTURE1); // activate texture unit 1 for mipmap ground texture
    glBindTexture(GL_TEXTURE_2D, g_tex_ground); // bind texture to active texture unit

    /*
     * TODO (Exercise 1.2)
     * BEGIN
     */

    // Load the available mip-map levels
    for (int lvl = 0; lvl < static_cast<int>(g_ground_textures.size()); ++lvl)
    {
        const Texture &tex = g_ground_textures[lvl];

        // TODO: Allocate and upload the texture data with glTexImage2D
		glTexImage2D(GL_TEXTURE_2D, lvl, GL_RGBA, tex.height(), tex.width(), 0, GL_BGRA, GL_FLOAT, tex.data());
    }
    // TODO: Generate the remaining mipmap levels
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, static_cast<int>(g_ground_textures.size() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<int>(g_ground_textures.size() - 1));
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    // TODO: Set the texture filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    /*
     * TODO (Exercise 1.2)
     * END
     */
}

/// Update the textures for the different ground materials
void UpdateGroundMaterialTextures()
{
    // Iterate over all material textures
    for (int i=0; i < g_material_textures.size(); ++i)
    {
        // Generate texture objects (but only once)
        if (g_material_texture_names[i] == 0)
            glGenTextures(1, &g_material_texture_names[i]);

        glActiveTexture(GL_TEXTURE2 + i); // activate texture units 2 and up for material textures
        glBindTexture(GL_TEXTURE_2D, g_material_texture_names[i]); // bind texture to active texture unit

        /*
        * TODO (Exercise 2.1)
        * BEGIN
        */

        const Texture &tex = g_material_textures[i];

        // TODO: Allocate and upload the texture data (for mip-map level 0) with glTexImage2D
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.height(), tex.width(), 0, GL_BGRA, GL_FLOAT, tex.data());
        // TODO: Generate the remaining mipmap levels
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        // TODO: Set the filters. Use the values from exercise 1.2 and set GL_TEXTURE_MIN_FILTER to GL_LINEAR_MIPMAP_LINEAR
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        /*
        * TODO (Exercise 2.1)
        * END
        */
    }
}

/// Initializes the intermediate framebuffers or updates them.
void UpdateFramebuffers()
{
    // update the texture for rendering the shadow map to
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_tex_shadowmap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, g_shadowmap_width, g_shadowmap_height, 0, GL_RED,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // update the depth buffer used for rendering the shadow map
    glBindTexture(GL_TEXTURE_2D, g_tex_shadowmap_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, g_shadowmap_width, g_shadowmap_height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // update the framebuffer for the 1st render pass where the shadow map is rendered
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer_shadowmap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_tex_shadowmap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           g_tex_shadowmap_depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Failed to update the shadowmap framebuffer.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// Creates geometry and uniform buffers.
void InitScene()
{
    g_render_batch_elephant = CreateElephant();
    g_render_batch_ground = CreateGround();
}

/// @return The elephant's geometry wrapped in a RenderBatch object.
RenderBatch CreateElephant()
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::tie(vertices, indices) = LoadObject("objects/elepham.obj");
    glm::mat4 transform = glm::scale(glm::mat4{}, glm::vec3{0.005f});
    return CreateRenderBatch(vertices, indices, false, transform);
}

/// @return The ground's geometry wrapped in a RenderBatch object.
RenderBatch CreateGround()
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 color{0.3f};

    vertices.reserve(g_ground_extent * g_ground_extent);
    indices.reserve((g_ground_extent - 1) * (g_ground_extent - 1) * 6);

    for (int x = 0; x < g_ground_extent; ++x)
    {
        for (int z = 0; z < g_ground_extent; ++z)
        {
            Vertex v;
            v.pos = {x, GetGroundHeight({x, z}), z};
            v.normal = GetGroundNormal({x, z});
            // v.color = v.normal;
            v.color = color;
            vertices.push_back(v);
        }
    }

    for (int x = 0; x < g_ground_extent - 1; ++x)
    {
        for (int z = 0; z < g_ground_extent - 1; ++z)
        {
            int a = (x * g_ground_extent) + z;
            int b = ((x + 1) * g_ground_extent) + z;
            int c = ((x + 1) * g_ground_extent) + (z + 1);
            int d = (x * g_ground_extent) + (z + 1);
            // first triangle
            indices.push_back(c);
            indices.push_back(b);
            indices.push_back(a);
            // second triangle
            indices.push_back(a);
            indices.push_back(d);
            indices.push_back(c);
        }
    }

    glm::vec3 translation{-g_ground_extent / 2.f, 0.01f, -g_ground_extent / 2.f};
    glm::mat4 transform = glm::translate(glm::mat4{}, translation);
    return CreateRenderBatch(vertices, indices, true, transform);
}

/// @param coord Coordinates on the ground's grid.
/// @return The disired height of the ground at that coordinate.
/// @throws std::runtime_error if coord contains a negative value or a value above g_ground_extent.
float GetGroundHeight(glm::ivec2 coord)
{
    static std::vector<std::array<float, g_ground_extent>> height_map;

    if (glm::clamp(coord, 0, g_ground_extent - 1) != coord)
        throw std::runtime_error("Index out of bounds.");
    if (height_map.empty())
    {
        height_map.resize(g_ground_extent);

        for (int x = 0; x < g_ground_extent; ++x)
        {
            for (int y = 0; y < g_ground_extent; ++y)
            {
                float noise = simplex::fractal2D(0.0158229f, // frequency
                                                 2.0f,       // lacunarity
                                                 0.4f,       // persistence
                                                 5,          // number of octaves
                                                 static_cast<float>(x),
                                                 static_cast<float>(y)); // coodinates
                noise = noise * 0.5f + 0.5f;
                noise *= noise;
                float hilliness = simplex::noise2D(0.011887f * x, 0.011887f * y);
                hilliness *= hilliness;
                hilliness = 1.0f - hilliness;
                hilliness *= hilliness;
                hilliness = 0.4f + 0.6f * hilliness;
                int dx = g_ground_extent / 2 - x;
                int dy = g_ground_extent / 2 - y;
                float plain = static_cast<float>(dx * dx + dy * dy);
                plain = std::min(plain / 400.f, 1.0f);
                plain = 1.0f - plain;
                plain *= plain;
                plain = 1.0f - plain;
                height_map[x][y] = g_ground_y_scale * noise * hilliness * plain;
            }
        }
    }
    return height_map[coord.x][coord.y];
}

/// @param coords Coordinates on the ground's grid.
/// @return The disired normal vector of the ground at that coordinate.
/// @throws std::runtime_error if coord contains a negative value or a value above g_ground_extent.
glm::vec3 GetGroundNormal(glm::ivec2 coords)
{
    static std::vector<std::array<glm::vec3, g_ground_extent>> normal_map;

    if (glm::clamp(coords, 0, g_ground_extent - 1) != coords)
        throw std::runtime_error("Index out of bounds.");

    if (normal_map.empty())
    {
        normal_map.resize(g_ground_extent);

        for (int x = 0; x < g_ground_extent; ++x)
        {
            for (int z = 0; z < g_ground_extent; ++z)
            {
                normal_map[x][z] = glm::vec3(0.f, 1.f, 0.f);

                // border cases
                if (x == 0 || z == 0 || x == g_ground_extent - 1 || z == g_ground_extent - 1)
                    continue;

                glm::vec3 p = {x, GetGroundHeight({x, z}), z};
                glm::vec3 u = {x, GetGroundHeight({x, z + 1}), z + 1};
                glm::vec3 ur = {x + 1, GetGroundHeight({x + 1, z + 1}), z + 1};
                glm::vec3 r = {x + 1, GetGroundHeight({x + 1, z}), z};
                glm::vec3 d = {x, GetGroundHeight({x, z - 1}), z - 1};
                glm::vec3 dl = {x - 1, GetGroundHeight({x - 1, z - 1}), z - 1};
                glm::vec3 l = {x - 1, GetGroundHeight({x - 1, z}), z};

                glm::vec3 normal = glm::cross(ur - p, r - p) + glm::cross(r - p, d - p) +
                                   glm::cross(d - p, dl - p) + glm::cross(dl - p, l - p) +
                                   glm::cross(l - p, u - p) + glm::cross(u - p, ur - p);
                normal /= 6.f;

                normal_map[x][z] = glm::normalize(normal);
            }
        }
    }

    return normal_map[coords.x][coords.y];
}

/// @param vertices A vertex buffer.
/// @param out_indices An index buffer.
/// @param use_texture Whether to use the bound g_ground_texture (sampled at world coords xy) to
/// render that object.
/// @param model The model matrix to use.
/// @return RenderBatch object created using the given buffers.
RenderBatch CreateRenderBatch(const std::vector<Vertex> &vertices,
                              const std::vector<uint32_t> &indices, bool use_texture,
                              const glm::mat4 &model)
{
    RenderBatch batch;
    batch.per_object.model = model;
    batch.per_object.use_texture = use_texture;
    batch.index_count = static_cast<int32_t>(indices.size());

    // create & fill index buffer object
    glGenBuffers(1, &batch.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(uint32_t) * indices.size(), // data size
                 indices.data(),                    // data pointer
                 GL_STATIC_DRAW);                   // usage
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // create vertex array object
    glGenVertexArrays(1, &batch.vao);
    glBindVertexArray(batch.vao);

    // create & fill vertex buffer object
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
        reinterpret_cast<const void *>(offsetof(Vertex, pos))); // offset of this attribute

    // specify & enable color attribute (the data is taken from the currently bound vbo)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,           // index of the attribute
        3, GL_FLOAT, // a color consists of 3 floats
        GL_FALSE,
        sizeof(Vertex), // stride between consecutive colors in the buffer
        reinterpret_cast<const void *>(offsetof(Vertex, color))); // offset of this attribute

    // specify & enable normal attribute (the data is taken from the currently bound vbo)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,           // index of the attribute
        3, GL_FLOAT, // a normal consists of 3 floats
        GL_FALSE,
        sizeof(Vertex), // stride between consecutive normals in the buffer
        reinterpret_cast<const void *>(offsetof(Vertex, normal))); // offset of this attribute

    // unbind bound objects
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return batch;
}

/// Check if the first joystick/gamepad slot (GLFW_JOYSTICK_1) is connected.
/// If it is, use its input to update camera yaw, pitch and position.
/// Tested with a PS4 Controller and DS4Windows. XboxOne controllers might experience inverted axes.
void applyJoystickInput()
{
    if (!glfwJoystickPresent(GLFW_JOYSTICK_1))
        return;

    g_t0 = g_t1;
    g_t1 = glfwGetTime();
    double dt = g_t1 - g_t0;

    int count;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

    // update yaw and pitch
    float rot_dx = axes[2];
    float rot_dy = axes[3];

    if (abs(rot_dx) < 0.025f)
        rot_dx = 0.0f;

    if (abs(rot_dy) < 0.025f)
        rot_dy = 0.0f;

    rot_dx = std::abs(rot_dx) * rot_dx * 1.5f;
    rot_dy = std::abs(rot_dy) * rot_dy * 1.5f;

    g_cam_yaw += -rot_dx * static_cast<float>(dt);
    g_cam_yaw = std::remainder(g_cam_yaw, 2 * glm::pi<float>());
    g_cam_pitch += rot_dy * static_cast<float>(dt);
    g_cam_pitch = glm::clamp(g_cam_pitch, -1.5f, 1.5f);


    float trans_dx = axes[0];
    float trans_dy = axes[1];

    if (abs(trans_dx) < 0.05f)
        trans_dx = 0.0f;

    if (abs(trans_dy) < 0.05f)
        trans_dy = 0.0f;

    glm::vec3 position = g_cam_pos;

    // calculate camera direction
    glm::vec3 vfront{ 0, 0, -1 };
    vfront = glm::mat3(glm::rotate(glm::mat4(), g_cam_pitch, glm::vec3(1, 0, 0))) * vfront;
    vfront = glm::mat3(glm::rotate(glm::mat4(), g_cam_yaw, glm::vec3(0, 1, 0))) * vfront;

    glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 vRighthand = glm::normalize(glm::cross(vfront, vUp));

    g_cam_pos = position + trans_dy * 20.0f * vfront * (float)dt + trans_dx * 20.0f * vRighthand * (float)dt;
}


/// Synchronizes scene updates to a frequenzy of 100 Hz.
/// This function assumes that the scene is updated each time it returns true.
/// @return true iff the scene should be updated now.
bool NeedSceneUpdate()
{
    static auto nextUpdate = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    if (nextUpdate > now)
        return false;
    auto maxBehind = now - std::chrono::milliseconds{100};
    nextUpdate = std::max(nextUpdate + std::chrono::milliseconds{10}, maxBehind);
    return true;
}

/// Applies per-frame updates of the scene.
void UpdateScene()
{
    applyJoystickInput();

    // update camera yaw and pitch
    int rot_horz_target = KeyAxisValue(g_window, GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
    int rot_vert_target = KeyAxisValue(g_window, GLFW_KEY_DOWN, GLFW_KEY_UP);
    g_diff_cam_yaw = lerp(0.1f, g_diff_cam_yaw, static_cast<float>(rot_horz_target));
    g_diff_cam_pitch = lerp(0.1f, g_diff_cam_pitch, static_cast<float>(rot_vert_target));
    g_cam_yaw += 0.02f * g_diff_cam_yaw;
    g_cam_yaw = std::remainder(g_cam_yaw, 2 * glm::pi<float>());
    g_cam_pitch += 0.02f * g_diff_cam_pitch;
    g_cam_pitch = glm::clamp(g_cam_pitch, -1.5f, 1.5f);

    // calculate camera direction
    glm::vec3 cam_dir{0, 0, -1};
    cam_dir = glm::mat3{glm::rotate(glm::mat4{}, g_cam_pitch, glm::vec3{1, 0, 0})} * cam_dir;
    cam_dir = glm::mat3{glm::rotate(glm::mat4{}, g_cam_yaw, glm::vec3{0, 1, 0})} * cam_dir;

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

    // calculate view matrix
    glm::mat4 view = glm::lookAt(g_cam_pos,           // camera position
                                 g_cam_pos + cam_dir, // point to look towards (not a direction)
                                 glm::vec3{0, 1, 0}); // up direction

    // calculate projection matrix
    float aspect = static_cast<float>(g_window_width) / g_window_height;
    glm::mat4 projection =
        glm::perspective(0.25f * glm::pi<float>(), // fov in y-direction in degrees
                         aspect,                   // aspect ratio w/h
                         0.1f,                     // distance to near plane
                         500.f);                   // distance to far plane

    // calculate light direction and position
    g_light_yaw += 0.004f * KeyAxisValue(g_window, GLFW_KEY_1, GLFW_KEY_2);
    g_light_yaw = std::remainder(g_light_yaw, 2 * glm::pi<float>());
    glm::vec3 light_pos;
    glm::vec3 light_dir = glm::normalize(glm::vec3{0.75f, -1.f, 0.f});
    light_dir = glm::mat3(glm::rotate(glm::mat4{}, g_light_yaw, glm::vec3{0, 1, 0})) * light_dir;

    // calculate view matrix for rendering the shadow map
    glm::mat4 shadowmap_view = glm::lookAt(light_pos,             // position of the light
                                           light_pos + light_dir, // point the light shines towards
                                           glm::vec3{0, 1, 0});   // up direction

    // calculate projection matrix for rendering the shadow map
    glm::mat4 shadowmap_projection = glm::ortho(-10.f, 10.f,  // bottom / top
                                                -10.f, 10.f,  // left / right
                                                -30.f, 30.f); // near / far

    // update the local copy of the per-frame uniform buffer
    g_per_frame.view = view;
    g_per_frame.proj = projection;
    g_per_frame.inv_view_proj = glm::inverse(projection * view);
    g_per_frame.shadowmap_view_proj = shadowmap_projection * shadowmap_view;
    g_per_frame.clip_coords_to_shadowmap =
        g_per_frame.shadowmap_view_proj * glm::inverse(projection * view);
    g_per_frame.light_dir = glm::vec4(glm::normalize(light_dir), 0.f);
    g_per_frame.resolution = {g_window_width, g_window_height};
    g_per_frame.shadowmap_resolution = {g_shadowmap_width, g_shadowmap_height};
}

/// Renders a frame.
void RenderFrame()
{
    // update per-frame uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_frame);
    void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(data, &g_per_frame, sizeof(g_per_frame));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // setup for rendering the shadow map
    glViewport(0, 0, g_shadowmap_width, g_shadowmap_height);
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer_shadowmap);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g_shader_program_shadowmap);
    // render the geometry
    RenderObject(g_render_batch_elephant);
    RenderObject(g_render_batch_ground);

    // setup for renering to the screen
    glViewport(0, 0, g_window_width, g_window_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.53f, 0.81f, 0.98f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_tex_shadowmap);

    glUseProgram(g_shader_program_final);

    // render elefant
    RenderObject(g_render_batch_elephant);

    // choose shader program for ground
    if (g_show_material_texture)
        glUseProgram(g_shader_program_ground_material);
    
    // render ground
    RenderObject(g_render_batch_ground);

    glUseProgram(0);
}

/// Renders a RenderBatch.
/// @param batch The RenderBatch to render.
void RenderObject(const RenderBatch &batch)
{
    // upload per-object data
    glBindBuffer(GL_UNIFORM_BUFFER, g_ubo_per_object);
    void *data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(data, &batch.per_object, sizeof(batch.per_object));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_ubo_per_object);

    // actually render
    glBindVertexArray(batch.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ibo);
    glDrawElements(GL_TRIANGLES, batch.index_count, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/// Updates the resolution. This is called by GLFW when the resolution changes.
/// @param window The window that changed resolution.
/// @param width New width.
/// @param height New height.
void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    if (width > 0 && height > 0)
    {
        g_window_width = width;
        g_window_height = height;
        UpdateFramebuffers();
    }
}

/// Reloads the shaders if 'R' is pressed. This is called by GLFW when a key is pressed.
/// @param window The window in which a key was pressed.
/// @param key GLFW key code of the pressed key.
/// @param scancode platform-specific key code.
/// @param action One of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
/// @param mods Modifier bits.
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_R:
            UpdateShaders();
            break;
        case GLFW_KEY_3:
            UpdateGroundTexture(GL_NEAREST);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_4:
            UpdateGroundTexture(GL_LINEAR);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_5:
            UpdateGroundTexture(GL_NEAREST_MIPMAP_NEAREST);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_6:
            UpdateGroundTexture(GL_LINEAR_MIPMAP_NEAREST);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_7:
            UpdateGroundTexture(GL_NEAREST_MIPMAP_LINEAR);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_8:
            UpdateGroundTexture(GL_LINEAR_MIPMAP_LINEAR);
            g_show_material_texture = false;
            break;
        case GLFW_KEY_9:
            g_show_material_texture = true;
            break;
        }
    }
}
