#pragma once
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct {
  GLFWwindow *window;
  uint32_t texture_id;
  uint32_t framebuffer_id;
  float zoom_factor;
  float x_offset, y_offset;
  uint32_t image_width, image_height;
  uint32_t screen_coords[4];
  uint32_t fb_coords[4];
  bool state_changed;
  char **image_paths;
  int image_count;
  int image_index;
} Renderer;
Renderer init_renderer(const char *path);
void render(Renderer *renderer);
void renderer_navigate(Renderer *renderer, int direction);
