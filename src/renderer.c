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
  uint32_t image_width = out.width;
  uint32_t image_height = out.height;

  window_width = image_width;
  window_height = image_height;
  printf("Detected image dimensions (%d, %d)\n", image_width, image_height);

  GLFWwindow *window = create_window(false);
  init_glad();

  Renderer renderer = {.window = window,
                       .texture_id = 0,
                       .framebuffer_id = 0,
                       .zoom_factor = 1.0f,
                       .x_offset = 0.0f,
                       .y_offset = 0.0f,
                       .image_width = image_width,
                       .image_height = image_height,
                       .screen_coords = {0, 0, window_width, window_height},
                       .fb_coords = {0, 0, image_width, image_height},
                       .state_changed = false};

  unsigned char *data;
  pthread_join(img_load_thread, (void **)&data);

  if (data == NULL) {
    fprintf(stderr, "Decoding image failed, exiting");
    exit(1);
  }

  renderer.texture_id = load_texture_framebuffer(
      (void *)out.output_buffer, &(renderer.framebuffer_id), image_width,
      image_height, out.gl_format);
  if (!(renderer.texture_id)) {
    fprintf(stderr, "Error: Loading the OpenGL texture failed!\n");
    exit(2);
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer.framebuffer_id);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  return renderer;
}

void render(Renderer *renderer) {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBlitFramebuffer(renderer->fb_coords[0], renderer->fb_coords[1],
                    renderer->fb_coords[2], renderer->fb_coords[3],
                    renderer->screen_coords[0], renderer->screen_coords[1],
                    renderer->screen_coords[2], renderer->screen_coords[3],
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glfwSwapBuffers(renderer->window);
}
