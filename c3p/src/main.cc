// expression templates
// constexpr
//
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <experimental/random>
#include <iostream>
#include <vector>
#include <thread>

#include <gtk/gtk.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <c3p/control_window.h>
#include <c3p/common/shader.h>
#include <c3p/particle_renderer.h>
#include <c3p/particle_system.h>
//#include <c3p/particle_container.h>

GLFWwindow* window;

bool mousedown;  // TODO eek no global variables!

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    mousedown = true;
  else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    mousedown = false;
}

int main(void)
{
  ControlData * ctl_p; //struct to hold the data from the control window
  struct ControlData ctl;
  ctl_p = &ctl;
  initControls(ctl_p);

  std::thread ctl_window ([ctl_p]{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), (gpointer)ctl_p);
    status = g_application_run(G_APPLICATION (app), 0, nullptr);
    g_object_unref(app);
      });

  std::thread view ([ctl_p]{

  // Initialise GLFW
  if (!glfwInit())
    {
      fprintf(stderr, "Failed to initialize GLFW\n");
      getchar();
      return -1;
    }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1280, 1400, "c3particles", NULL, NULL);

  if (window == NULL)
    {
      fprintf(stderr,
              "Failed to open GLFW window. If you have an Intel GPU, they are "
              "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
      getchar();
      glfwTerminate();
      return -1;
    }
  glfwMakeContextCurrent(window);
glewExperimental = true;  // Needed for core profile Initialize GLEW
  if (glewInit() != GLEW_OK)
    {
      fprintf(stderr, "Failed to initialize GLEW\n");
      getchar();
      glfwTerminate();
      return -1;
    }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // black background
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  GLuint shaders = LoadShaders("../shaders/simple_vert_shader.glsl",
                               "../shaders/simple_frag_shader.glsl");

  // Get a handle for shaders
  GLuint MatrixID = glGetUniformLocation(shaders, "MVP");

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit
  // <-> 100 units
  glm::mat4 Projection =
      glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
  // Or, for an ortho camera :
  // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);
  // // In world coordinates

  // Camera matrix
  glm::mat4 View = glm::lookAt(
      glm::vec3(0, 0, -200),  // Camera is at (x,y,z), in World Space
      glm::vec3(0, 0, 0),     // and looks at the origin
      glm::vec3(0, 1, 0)      // Head is up (set to 0,-1,0 to look upside-down)
      );
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 mvp =
      Projection * View *
      Model;  // Remember, matrix multiplication is the other way around

  glPointSize(5.0f);
  int width, height;
  double xpos, ypos;
  c3p::ParticleSystem particles(3);
  particles.setRandom();
  c3p::ParticleRenderer p_renderer(particles);

//  c3p::ParticleSystem particles2(100);
//  particles2.setRandom();
//  c3p::ParticleRenderer p_renderer2(particles2);

//  auto pc = c3p::ParticleContainer();

  do
    {

//      std::cout << ctl_p->zoom_scale << std::endl;
      // clear the screen and clear the depth
      //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glClear(GL_DEPTH_BUFFER_BIT);
      // Enable depth test
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glEnable(GL_BLEND);
      // Accept fragment if it closer to the camera than the former one
      glDepthFunc(GL_LESS);

      // use own shader
      glUseProgram(shaders);

      // calculate mouse position in world space
      glfwGetFramebufferSize(window, &width, &height);
      glfwGetCursorPos(window, &xpos, &ypos);
      xpos -= width / 2;
      ypos -= height / 2;

      // dirty hack to scale movement. TODO invert projection matrix or
      // something
      xpos /= 100;
      ypos /= 100;

      // physics engine in own thread --> sleep
      // measure time since last swap buffers (std::chrono)
      glm::vec3 random = {rand() / (float)RAND_MAX, rand() / (float)RAND_MAX,
                          rand() / (float)RAND_MAX};

      //            particles.applyForceAll(vec3(0.01,0.0,0.0));
      if (mousedown) {particles.addAttractor(glm::vec3(-xpos, -ypos, 0.0), 1.0);}
      //            else
      //                particles.gravitateOrigin(0.7);
      //               particles.applyForceAll(random);

      //            particles.addAttractor(glm::vec3{0.0,0,0}, 1.0);
      particles.addGForce(glm::vec3{0, 0, 0}, 50);
      //            particles.addGForce(glm::vec3{-50,0,0}, 50);
      particles.nbodyGravity();
      particles.update();
      p_renderer.render(mvp, MatrixID);

//      particles2.addGForce(glm::vec3{0, 0, 0}, 100);
//      //            particles.addGForce(glm::vec3{-50,0,0}, 50);
//      particles2.nbodyGravity();
//      particles2.update();
//      p_renderer2.render(mvp, MatrixID);
//      particles.print();

      // Swap buffers

      glfwSwapBuffers(window);
      glfwPollEvents();

    }  // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
      });

  ctl_window.join();
  view.join();

  return 0;
}
