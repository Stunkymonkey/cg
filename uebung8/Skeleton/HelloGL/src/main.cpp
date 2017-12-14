// OpenGL example application, that renders a triangle on the screen.

// OpenGL loader
#include <glad/glad.h>

// OpenGL framework
#include <GLFW/glfw3.h>

#include <array>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <vector>

void InitRenderer();
void RenderFrame();
void UpdateTransform();
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
std::string ReadFile(const char *path);
GLint CompileShader(GLenum shader_type, const char *file_path);
std::string GetShaderError(GLuint shader);
GLint LinkProgram(const std::vector<GLuint> &shaders);
std::string GetProgramError(GLuint program);

GLFWwindow *g_window;                                           // GLFW window handle
static int g_window_width = 960, g_window_height = 540;         // initial window resolution
static GLuint g_triangle_vao, g_triangle_vbo, g_shader_program; // used for rendering
const static int g_vertex_count = 3 * 4; // star made of 4 triangles a 3 vertices

/// RAII class for initializing and terminating GLFW.
/// This is a singleton since GLFW should be initialized exactly once.
class GlfwInstance
{
  public:
    /// Ensures that GLFW is initialized.
    static void init() { static GlfwInstance instance; }

  private:
    /// Constructor initializes GLFW.
    GlfwInstance()
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW.");
    }

    /// Destructor terminates GLFW.
    ~GlfwInstance() { glfwTerminate(); }
};

/// Entry point into the OpenGL example application.
int main()
{
    try
    {
        // initialize GLFW using our own singleton RAII class
        GlfwInstance::init();

        // initialize renderer
        InitRenderer();

        // render until the window should close
        while (!glfwWindowShouldClose(g_window))
        {
            glfwPollEvents();          // handle events
            RenderFrame();             // render image
            glfwSwapBuffers(g_window); // present image
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\nPress enter.\n";
        std::cin.get();
        return -1;
    }
}

/// Initializes the renderer.
/// @throw std::runtime_error If an error occurs during intialization.
void InitRenderer()
{
    // create window
    g_window = glfwCreateWindow(g_window_width, g_window_height, // initial resolution
                                "Hello, OpenGL",                 // window title
                                nullptr, nullptr);
    if (!g_window)
        throw std::runtime_error("Failed to create window.");

    // use the window that was just created
    glfwMakeContextCurrent(g_window);

    // get window resolution (since there are cases where the created window doesn't have the
    // requested resolution)
    glfwGetFramebufferSize(g_window, &g_window_width, &g_window_height);

    // set a callback that is called when the resolution changes
    glfwSetFramebufferSizeCallback(g_window, &FramebufferSizeCallback);

    // load OpenGL (return value of 0 indicates error)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Failed to load OpenGL context.");
    std::cout << "OpenGL " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

    // compile vertex & fragment shader
    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, "shaders/vert.glsl");
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, "shaders/frag.glsl");

    // link shaders
    g_shader_program = LinkProgram({vertex_shader, fragment_shader});

    // clean shaders up
    glDetachShader(g_shader_program, fragment_shader);
    glDetachShader(g_shader_program, vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    // create & bind vertex array object that holds the state
    glGenVertexArrays(1, &g_triangle_vao);
    glBindVertexArray(g_triangle_vao);

    // setup vertices for the star shape
    std::array<glm::vec3, g_vertex_count> vertices;
    // equilateral triangle with center at (0,0)
    vertices[0] = {-0.5f, -sqrt(3.f) / 6.f, 0.f};
    vertices[1] = {0.5f, -sqrt(3.f) / 6.f, 0.f};
    vertices[2] = {0.0f, sqrt(3.f) / 3.f, 0.f};
    // rotation the other 3 triangles 30, 60 and 90 degrees
    const float angle = glm::radians(30.f);
    for (int i = 3; i < g_vertex_count; ++i)
        vertices[i] = glm::rotateZ(vertices[i % 3], (i / 3) * angle);

    /*
     * TODO (zu Teilaufgabe 2): Nutzen Sie die OpenGL-Funktion glGenBuffer, um ein Vertex Buffer
     * Object (das einen Buffer auf der GPU repräsentiert) zu erstellen. Das Handle zum Buffer soll
     * dabei in g_triangle_vbo abgelegt werden.
     */

    /*
     * TODO (zu Teilaufgabe 2): Nutzen Sie die OpenGL-Funktionen glBindBuffer und glBufferData, um
     * den Inhalt des erstellten 'vertices' Arrays in den erstellten Buffer auf die GPU zu laden.
     */

    /*
     * TODO (zu Teilaufgabe 2): Nutzen Sie die OpenGL-Funktionen glEnableVertexAttribArray und
     * glVertexAttribPointer, sodass der Vertex-Shader auf die Positionen der Vertices der Dreiecke
     * zugreifen kann.
     */

    // unbind vertex array object & vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // upload the transform matrix
    UpdateTransform();
}

/// Renders a frame.
void RenderFrame()
{
    // render settings
    glViewport(0, 0, g_window_width, g_window_height);
    glUseProgram(g_shader_program);

    // clear & draw to the framebuffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(g_triangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, g_vertex_count);
    glBindVertexArray(0);

    glUseProgram(0);
}

/// Updates the transform matrix.
/// To counteract window resizing, it scales the x-component by the inverse
/// aspect ratio.
void UpdateTransform()
{
    // specify the shader program that will use the transform matrix
    glUseProgram(g_shader_program);

    /*
     * TODO (zu Teilaufgabe 3): Erstellen Sie eine Matrix und übergeben Sie diese unter Verwendung
     * der OpenGL-Funktion glUniformMatrix4fv an den Shader. Diese soll dabei den in der
     * Aufgabenstellung spezifizierten Effekt erzielen.
     */

    glUseProgram(0);
}

/// Updates the resolution. This is called by GLFW when the resolution changes.
/// @param window The window that changed resolution
/// @param width New width.
/// @param height New height.
void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    g_window_width = width;
    g_window_height = height;
    UpdateTransform();
}

/// Reads a file.
/// @param path Path to the file.
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

/// Creates and compiles a shader using the shader code contained in a file.
/// @param shader_type Type of the shader.
/// @param file_path Path to the file containing the shader code.
/// @return Handle of the compiled shader.
/// @throw std::runtime_error if compilation failed.
GLint CompileShader(GLenum shader_type, const char *file_path)
{
    // read the source code from the file
    std::string code = ReadFile(file_path);
    const char *p_code = code.c_str(); // Null-terminated string containing the shader code

    /*
     * TODO (zu Teilaufgabe 1): Benutzen Sie die OpenGL-Funktionen glCreateShader, glShaderSource
     * und glCompileShader, um den in p_code enthaltenen Shader-Code zu kompilieren. Der kompilierte
     * Shader soll vom Typ shader_type sein und sein Handle soll in der Variable shader abgelegt
     * werden.
     *
     * Hinweis: Sollten Sie einen Pointer auf p_code an eine OpenGL-Funktion übergeben
     * müssen, erhalten Sie diesen durch &p_code
     */
    GLint shader = 0; // TODO: set shader properly

    // check if compiling succeeded
    GLint compile_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
    if (compile_success == 0)
        throw std::runtime_error(std::string("Failed to compile shader \"") + file_path + "\".\n" +
                                 GetShaderError(shader));
    return shader;
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

/// Creates and links a shader program.
/// @param shaders Handles of the shaders to attach before linking.
/// @return Handle of the linked program.
/// @throw std::runtime_error if linking failed.
GLint LinkProgram(const std::vector<GLuint> &shaders)
{
    /*
     * TODO (zu Teilaufgabe 1):
     * Erstellen Sie das Programm und hängen Sie alle Shader an. Vergessen Sie nicht, das Programm
     * zu linken.
     */
    GLint program = 0; // TODO: set program properly

    // check if linking succeeded
    GLint link_success;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (link_success == 0)
        throw std::runtime_error("Failed to link shader program.\n" + GetProgramError(program));
    return program;
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
