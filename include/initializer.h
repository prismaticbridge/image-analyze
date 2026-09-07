#pragma once
#include <GLFW/glfw3.h>
#include <stdbool.h>

void init_glfw(int major_version, int minor_version);
void init_glad();
GLFWwindow *create_window(bool fullscreen);
uint32_t load_texture_framebuffer(unsigned char *data, uint32_t *write_framebuffer,
                                uint32_t width, uint32_t height,
                                GLenum internal_format);
