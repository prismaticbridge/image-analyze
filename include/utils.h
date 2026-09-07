#pragma once
#include <stdint.h>
inline float min(float a, float b);
inline float max(float a, float b);
void calculate_framebuffer_coords(uint32_t write_framebuffer_coords[4], float offset_x, float offset_y, int32_t framebuffer_width, int32_t framebuffer_height, float zoom_factor);
void calculate_screen_coords(uint32_t write_screen_coords[4], float offset_x, float offset_y, int32_t framebuffer_width, int32_t framebuffer_height, float zoom_factor);


