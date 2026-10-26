#include <constants.h>
#include <decoders.h>
#include <filemanager.h>
#include <glad/gl.h>
#include <initializer.h>
#include <pthread.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
// #include <time.h>

static const char *path_basename(const char *path) {
  const char *slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

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
  glfwSetWindowTitle(window, path_basename(path));
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
                       .state_changed = false,
                       .image_paths = NULL,
                       .image_count = 0,
                       .image_index = 0};

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

  // Build an alphabetical list of the images in this image's directory.
  renderer.image_paths = list_image_files(path, &renderer.image_count);
  if (renderer.image_paths != NULL) {
    const char *base = path_basename(path);
    for (int i = 0; i < renderer.image_count; i++) {
      if (!strcmp(path_basename(renderer.image_paths[i]), base)) {
        renderer.image_index = i;
        break;
      }
    }
  }

  return renderer;
}

static bool renderer_show_image(Renderer *renderer, int index) {
  if (renderer->image_paths == NULL || index < 0 ||
      index >= renderer->image_count)
    return false;

  const char *path = renderer->image_paths[index];
  OutputBuffer out = {.output_buffer = NULL};
  LoadArgs args = {path, &out};

  if (load_image_thread(&args) == NULL || out.output_buffer == NULL ||
      out.width == 0 || out.height == 0) {
    fprintf(stderr, "Failed to load %s\n", path);
    return false;
  }

  // The texture is attached to the read framebuffer, so detach it first.
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  upload_texture(renderer->texture_id, (const unsigned char *)out.output_buffer,
                 out.width, out.height, out.gl_format);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->framebuffer_id);

  // HEIF decoding points the output buffer into libheif-owned memory.
  if (out.type != IMAGE_HEIC)
    free(out.output_buffer);

  renderer->image_index = index;
  renderer->image_width = out.width;
  renderer->image_height = out.height;
  renderer->zoom_factor = 1.0f;
  renderer->x_offset = ((float)window_width - (float)out.width) / 2.0f;
  renderer->y_offset = ((float)window_height - (float)out.height) / 2.0f;
  glfwSetWindowTitle(renderer->window, path_basename(path));
  renderer->state_changed = true;
  return true;
}

void renderer_navigate(Renderer *renderer, int direction) {
  if (renderer->image_count < 2)
    return;

  int index = (renderer->image_index + direction) % renderer->image_count;
  if (index < 0)
    index += renderer->image_count;

  renderer_show_image(renderer, index);
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
