#include <GLFW/glfw3.h>
#include <constants.h>
#include <inputHandler.h>
#include <utils.h>
// #include <stdio.h>

static user_pointer ptr;

void update_coords(Renderer *renderer) {
  calculate_framebuffer_coords(renderer->fb_coords, renderer->x_offset,
                               renderer->y_offset, renderer->image_width,
                               renderer->image_height, renderer->zoom_factor);
  calculate_screen_coords(renderer->screen_coords, renderer->x_offset,
                          renderer->y_offset, renderer->image_width,
                          renderer->image_height, renderer->zoom_factor);
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
  user_pointer *ptr = glfwGetWindowUserPointer(window);
  Renderer *renderer = ptr->renderer;

  double mouse_pos_x, mouse_pos_y;
  glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
  mouse_pos_y = window_height - mouse_pos_y;
  float mouse_offset_x = mouse_pos_x - renderer->x_offset;
  float mouse_offset_y = mouse_pos_y - renderer->y_offset;

  float zoom_offset_diff_x = mouse_offset_x * y_offset * 0.05;
  float zoom_offset_diff_y = mouse_offset_y * y_offset * 0.05;

  renderer->zoom_factor += y_offset * 0.05 * renderer->zoom_factor;

  renderer->x_offset -= zoom_offset_diff_x;
  renderer->y_offset -= zoom_offset_diff_y;

  renderer->state_changed = true;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);

  window_width = width;
  window_height = height;

  user_pointer *ptr = glfwGetWindowUserPointer(window);
  Renderer *renderer = ptr->renderer;

  renderer->state_changed = true;
}

void mouse_position_callback(GLFWwindow *window, double x_pos, double y_pos) {
  int mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  user_pointer *ptr = glfwGetWindowUserPointer(window);

  if (mouse_state == GLFW_PRESS) {
    Renderer *renderer = ptr->renderer;
    renderer->x_offset += x_pos - ptr->mouse_x;
    renderer->y_offset -= y_pos - ptr->mouse_y;

    renderer->state_changed = true;
  }
  ptr->mouse_x = x_pos;
  ptr->mouse_y = y_pos;
}

void init_input(Renderer *renderer) {
  glfwSetWindowUserPointer(renderer->window, &ptr);
  ptr.renderer = renderer;
  double mouse_x, mouse_y;
  glfwGetCursorPos(renderer->window, &mouse_x, &mouse_y);
  ptr.mouse_x = mouse_x;
  ptr.mouse_y = mouse_y;
  glfwSetCursorPosCallback(renderer->window, mouse_position_callback);
  glfwSetScrollCallback(renderer->window, scroll_callback);
  glfwSetFramebufferSizeCallback(renderer->window, framebuffer_size_callback);
}
