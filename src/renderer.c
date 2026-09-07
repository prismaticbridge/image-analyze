#include <constants.h>
#include <decoders.h>
#include <glad/gl.h>
#include <initializer.h>
#include <pthread.h>
#include <renderer.h>
#include <stdio.h>
#include <utils.h>
// #include <time.h>

Renderer init_renderer(const char *path) {
  // Start decoding image in parallel with opengl init stuff
  OutputBuffer out = {.output_buffer = NULL}; // leave other fields
                                              // uninitialized
  LoadArgs args = {
      path,
      &out,
  };
  pthread_t img_load_thread;
  pthread_create(&img_load_thread, NULL, load_image_thread, &args);

  init_glfw(3, 1);
  // spinwait for image width and height to be known
  while (out.width == 0 || out.height == 0) {
  }
  window_width = out.width;
  window_height = out.height;
  printf("Detected image dimensions (%d, %d)\n", window_width, window_height);

  GLFWwindow *window = create_window(false);
  init_glad();

  Renderer renderer = {window,
                       0,
                       0,
                       1.0f,
                       0.0f,
                       0.0f,
                       window_width,
                       window_height,
                       {0, 0, window_width, window_height},
                       {0, 0, window_width, window_height},
                       false};

  unsigned char *data;
  pthread_join(img_load_thread, (void **)&data);

  if (data == NULL) {
    fprintf(stderr, "Decoding image failed, exiting");
    exit(1);
  }

  renderer.textureId = load_texture_framebuffer(
      (void *)out.output_buffer, &(renderer.framebufferId), window_width,
      window_height, out.gl_format);
  if (!(renderer.textureId)) {
    fprintf(stderr, "Error: Loading the OpenGL texture failed!\n");
    exit(2);
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer.framebufferId);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  return renderer;
}

void render(Renderer *renderer) {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBlitFramebuffer(renderer->fbCoords[0], renderer->fbCoords[1],
                    renderer->fbCoords[2], renderer->fbCoords[3],
                    renderer->screenCoords[0], renderer->screenCoords[1],
                    renderer->screenCoords[2], renderer->screenCoords[3],
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glfwSwapBuffers(renderer->window);
}
