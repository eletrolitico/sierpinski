#include <GL/glew.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

float points[] = {
    0.0f, 0.0f, 0.0f,
    0.5f, 0.866f, 0.0f, 
    1.0f, 0.0f, 0.0f
};

static const char *vertex_shader_text =
    "#version 400\n"
    "in vec3 vPos;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 400\n"
    "out vec4 frag_colour;\n"
    "void main()\n"
    "{\n"
    "    frag_colour = vec4(0.5, 0.0, 0.0, 1.0);\n"
    "}\n";

void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

float dests[3][2] = {
    {-0.5f, 0.0f},
    {0.0f, 0.866f},
    {0.5f, 0.0f}
};

void get_next(float* cur){
    int dest = rand()%3;
    cur[0] = (dests[dest][0]+cur[0])/2;
    cur[1] = (dests[dest][1]+cur[1])/2;
}

static float tri_scale = 0.005;

int main(void) {
  GLFWwindow *window;
  GLuint vbo, vao, vertex_shader, fragment_shader, program;
  GLint mvp_location;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    die("glfw init");

  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1280, 720, "Sierpinski", NULL, NULL);
  if (!window) {
    glfwTerminate();
    die("glfw window");
  }

  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    die("glew init failed");
  }

  // get version info
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  mvp_location = glGetUniformLocation(program, "MVP");


  while (!glfwWindowShouldClose(window)) {
    float ratio;
    int width, height;
    mat4x4 m, p, mvp;
    vec2 last_point = {0.5, 0.0};
    int amount = glfwGetTime()*100;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(vao);

    mat4x4_identity(mvp);
    mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);

    srand(42);
    while(amount--){
        mat4x4_translate(m, last_point[0], last_point[1]-0.433, 0);
        mat4x4_scale_aniso(m, m, tri_scale, tri_scale, tri_scale);
        mat4x4_mul(mvp, p, m);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        get_next(last_point);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}

