/**
 * webgl firefox (windows) bug
 *
 * to compile with emscripten:
 *
 *  $ cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
 *  $ cmake --build .
 *
 */

// bug for webgl
#define USE_BUG_CASE



#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif



// draw call is a triangle strip with 4 triangles
// we use gl_VertexID to define the quad with vertices (half screen)
// in the bug case, the "foo" function is above the input variables, else below
#ifdef USE_BUG_CASE
const char *VERTEX_SHADER =
        "precision mediump float;\n"
        "const vec4 vertices[4] = vec4[](\n"
        "vec4(-0.5, +0.5, 0, 1),\n"
        "vec4(+0.5, +0.5, 0, 1),\n"
        "vec4(-0.5, -0.5, 0, 1),\n"
        "vec4(+0.5, -0.5, 0, 1)\n"
        ");\n"
        "vec4 foo()\n"
        "{\n"
        "  return vertices[gl_VertexID];\n"
        "}\n"
        "layout (location = 0) in mat4 in_matrix;\n"
        "uniform mat4 u_vp;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = foo();\n"
        "  gl_Position.x += in_matrix[0][0]*0.0001;\n"
        "}\n";
#else
const char *VERTEX_SHADER =
        "precision mediump float;\n"
        "const vec4 vertices[4] = vec4[](\n"
        "vec4(-0.5, +0.5, 0, 1),\n"
        "vec4(+0.5, +0.5, 0, 1),\n"
        "vec4(-0.5, -0.5, 0, 1),\n"
        "vec4(+0.5, -0.5, 0, 1)\n"
        ");\n"
        "layout (location = 0) in mat4 in_matrix;\n"
        "vec4 foo()\n"
        "{\n"
        "  return vertices[gl_VertexID];\n"
        "}\n"
        "uniform mat4 u_vp;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = foo();\n"
        "  gl_Position.x += in_matrix[0][0]*0.0001;\n"
        "}\n";
#endif


// just render white pixels
const char *FRAGMENT_SHADER =
        "precision mediump sampler2D;\n"
        "precision mediump float;\n"
        "out vec4 f_rgba;\n"
        "void main()\n"
        "{\n"
        "  f_rgba = vec4(1.0);\n"
        "}\n";

static void exit_fail(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

// local data for the main loop
static struct {
    SDL_Window *window;
    bool running;
    bool init_gl;
    GLuint program;
    GLuint vao;
} local;


// runs on first loop in the main loop
static void init_gl()
{
    local.init_gl = true;

    //
    // shader program
    //

    const char *version;
#ifdef __EMSCRIPTEN__
    version = "#version 300 es\n";
#else
    version = "#version 330 core\n";
#endif

    int status;
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 2, (const char *[]){version, VERTEX_SHADER}, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        exit_fail("glCompileShader GL_VERTEX_SHADER failed");
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 2, (const char *[]){version, FRAGMENT_SHADER}, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        exit_fail("glCompileShader GL_FRAGMENT_SHADER failed");
    }

    local.program = glCreateProgram();
    glAttachShader(local.program, vertex);
    glAttachShader(local.program, fragment);
    glLinkProgram(local.program);
    glGetProgramiv(local.program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        exit_fail("glLinkProgram failed");
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);


    //
    // buffer data
    //

    glGenVertexArrays(1, &local.vao);
    glBindVertexArray(local.vao);

}

static void main_loop(void)
{
    if(!local.init_gl) {
        init_gl();
    }

    // poll events to update sdl states
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            local.running = false;
            return;
        }
    }

    // update viewport
    int w_cols, w_rows;
    SDL_GL_GetDrawableSize(local.window, &w_cols, &w_rows);
    glViewport(0, 0, w_cols, w_rows);


    static float red = 0.0f;
    red += 0.001f;
    if(red >= 1.0f) {
        red = 0.0f;
    }

    // Render here, e.g., clear the buffer
    glClearColor(red, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the shader program
    glUseProgram(local.program);
    // Bind the VAO
    glBindVertexArray(local.vao);

    // Draw the 2 triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Swap front and back buffers
    SDL_GL_SwapWindow(local.window);
}

int main()
{
    printf("Hello, World!\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        exit_fail("SDL_Init failed");
    }

#ifdef __EMSCRIPTEN__
    printf("WebGL GLES 3.0\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    printf("OpenGL 3.3\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    local.window = SDL_CreateWindow("WebglTest",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          360 * 16 / 9, 360,
                                          SDL_WINDOW_OPENGL
                                          | SDL_WINDOW_RESIZABLE
    );
    if(!local.window) {
        exit_fail("SDL_CreateWindow failed");
    }

    if(!SDL_GL_CreateContext(local.window)) {
        exit_fail("SDL_GL_CreateContext failed");
    }

    const char* version = (const char*) glGetString(GL_VERSION);
    printf("OpenGL Version: %s\n", version);
    const char* vendor = (const char*) glGetString(GL_VENDOR);
    printf("GPU Vendor: %s\n", vendor);

    local.running = true;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (local.running) {
        main_loop();
    }
#endif

    return 0;
}
