#include <constants.h>
#include <stdint.h>
#include <utils.h>

float min(float a, float b) { return (a < b) ? a : b; }

float max(float a, float b) { return (a > b) ? a : b; }

void calculate_screen_coords(uint32_t write_screen_coords[4], float x_offset,
                             float y_offset, int32_t framebuffer_width,
                             int32_t framebuffer_height, float zoom_factor) {
  write_screen_coords[0] = max(0, x_offset);
  write_screen_coords[1] = max(0, y_offset);
  write_screen_coords[2] =
      min((float)window_width, x_offset + framebuffer_width * zoom_factor);
  write_screen_coords[3] =
      min((float)window_height, y_offset + framebuffer_height * zoom_factor);
}

void calculate_framebuffer_coords(uint32_t write_framebuffer_coords[4],
                                  float x_offset, float y_offset,
                                  int32_t framebuffer_width,
                                  int32_t framebuffer_height,
                                  float zoom_factor) {
  write_framebuffer_coords[0] = max(0.0f, -x_offset / zoom_factor);
  write_framebuffer_coords[1] =
      framebuffer_height - max(0.0f, -y_offset / zoom_factor);
  write_framebuffer_coords[2] =
      min((float)framebuffer_width, (window_width - x_offset) / zoom_factor);
  write_framebuffer_coords[3] =
      framebuffer_height -
      min((float)framebuffer_height, (window_height - y_offset) / zoom_factor);
}
