#include <GL/glew.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

mat4x4 mvp;
GLint mvp_loc;
int max_level=0;

// pontos para representar um triangulo equilátero
float points[] = {0.0f, 0.0f, 0.0f, 0.5f, 0.866f, 0.0f, 1.0f, 0.0f, 0.0f};

static const char *vertex_shader_text =
    "#version 400\n"
    "in vec3 vPos;\n"
    "uniform mat4 MVP;\n"
    "out vec2 col;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    col = (gl_Position.xy+1)/2;\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 400\n"
    "in vec2 col;\n"
    "uniform bool black;\n"
    "out vec4 frag_colour;\n"
    "void main()\n"
    "{\n"
    "   if(black){\n"
    "    frag_colour = vec4(0.1, 0.1, 0.11, 1.0);\n"
    "   } else {\n"
    "    frag_colour = vec4(col.x,col.y,0.5,1.0);\n"
    "   }\n"
    "}\n";

void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    max_level++;
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && max_level)
    max_level--;
}

GLuint create_shader(const char *const *src, GLuint type) {
  int shader = glCreateShader(type);
  glShaderSource(shader, 1, src, NULL);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

  if (isCompiled == GL_TRUE)
    return shader;

  GLint maxLength = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

  char *errorLog = malloc(maxLength);
  glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);
  printf("%s", errorLog);
  free(errorLog);

  exit(1);
}

void draw_tri(float x, float y, mat4x4 p, double scale) {
  mat4x4 m;
  mat4x4_translate(m, x, y, 0);
  mat4x4_scale_aniso(m, m, scale, scale, scale);
  mat4x4_mul(mvp, p, m);

  glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (const GLfloat *)mvp);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void recurse_tri(double x, double y, mat4x4 p, double scale, int level) {
  draw_tri(x, y, p, scale);

  if(!level--)
    return;

  scale /= 2;

  recurse_tri(x-scale/2,       y+scale*0.866, p, scale, level);// left
  recurse_tri(x+scale/2,       y-scale*0.866, p, scale, level);// top
  recurse_tri(x+scale/2+scale, y+scale*0.866, p, scale, level);// right
}

int main(void) {
  GLFWwindow *window;
  GLuint vbo, vao, vertex_shader, fragment_shader, program;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    die("glfw init");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

  vertex_shader = create_shader(&vertex_shader_text, GL_VERTEX_SHADER);
  fragment_shader = create_shader(&fragment_shader_text, GL_FRAGMENT_SHADER);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  mvp_loc = glGetUniformLocation(program, "MVP");
  GLint blk_loc = glGetUniformLocation(program, "black");
  mat4x4_identity(mvp);
  glClearColor(0.1, 0.1, 0.11, 0);

  while (!glfwWindowShouldClose(window)) {
    float ratio;
    int width, height;
    mat4x4 p;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(vao);

    glUniform1i(blk_loc, 1);
    mat4x4_ortho(p, -ratio, ratio, 1.f, -1.f, 1.f, -1.f);
    recurse_tri(-0.5, 0, p, 1, max_level);

    glUniform1i(blk_loc, 0);
    mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    draw_tri(-1, -0.866, p, 2);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
