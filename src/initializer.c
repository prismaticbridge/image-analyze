#include <constants.h>
#include <glad/gl.h>
#include <initializer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t window_width;
uint32_t window_height;

void init_glfw(int major_version, int minor_version) {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
  glfwInitHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
}

GLFWwindow *create_window(bool fullscreen) {
  GLFWmonitor *mon = glfwGetPrimaryMonitor();
  const GLFWvidmode *vid_mode = glfwGetVideoMode(mon);
  GLFWwindow *window;
  if (fullscreen) {
    window =
        glfwCreateWindow(vid_mode->width, vid_mode->height, "OpenGL", mon, NULL);
    window_width = vid_mode->width;
    window_height = vid_mode->height;
  } else {
    window_width = (window_width > vid_mode->width) ? vid_mode->width : window_width;
    window_height =
        (window_height > vid_mode->height) ? vid_mode->height : window_height;
    window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);
  }
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window\n");
    printf("Width: %d, Height: %d\n", window_width, window_height);
    glfwTerminate();
    exit(2);
  }
  // Create an OpenGL contex for the window
  glfwSwapInterval(1); // Limit framerate
  glfwMakeContextCurrent(window);
  return window;
}

void init_glad() {
  gladSetGLOnDemandLoader((GLADloadfunc)glfwGetProcAddress);
  if (glGetString == NULL) {
    fprintf(stderr, "Failed to load GL on-demand functions!\n");
    exit(1);
  }
  glViewport(0, 0, window_width, window_height);
}

static GLenum texture_pixel_format(GLenum internal_format) {
  switch (internal_format) {
  case GL_RGBA8:
  case GL_RGBA16:
    return GL_RGBA;
  case GL_RGB8:
  default:
    return GL_RGB;
  }
}

void upload_texture(uint32_t texture_id, const unsigned char *data,
                    uint32_t width, uint32_t height, GLenum internal_format) {
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // Fix alignment for non-4-byte row size
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
               texture_pixel_format(internal_format), GL_UNSIGNED_BYTE, data);
}

uint32_t load_texture_framebuffer(unsigned char *data, uint32_t *write_framebuffer,
                                uint32_t width, uint32_t height,
                                GLenum internal_format) {
  uint32_t texture_id;
  glGenTextures(1, &texture_id);
  upload_texture(texture_id, data, width, height, internal_format);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, write_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, *write_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_id, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Error: GL Framebuffer incomplete\n");
    return 0;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return texture_id;
}
