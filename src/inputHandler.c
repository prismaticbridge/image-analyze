#include <inputHandler.h>
#include <GLFW/glfw3.h>
#include <utils.h>
#include <constants.h>
//#include <stdio.h>

static userPointer ptr;

void update_coords(Renderer* renderer) {
    calculate_framebuffer_coords(renderer->fbCoords, renderer->xOffset, renderer->yOffset, renderer->framebufferWidth, renderer->framebufferHeight, renderer->zoomFactor);
    calculate_screen_coords(renderer->screenCoords, renderer->xOffset, renderer->yOffset, renderer->framebufferWidth, renderer->framebufferHeight, renderer->zoomFactor);
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    userPointer* ptr = glfwGetWindowUserPointer(window);
    Renderer* renderer = ptr->renderer;

    double mouse_pos_x, mouse_pos_y;
    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
    mouse_pos_y = window_height - mouse_pos_y;
    float mouse_offset_x = mouse_pos_x - renderer->xOffset;
    float mouse_offset_y = mouse_pos_y - renderer->yOffset; 

    float zoom_offset_diff_x = mouse_offset_x * y_offset * 0.05;
    float zoom_offset_diff_y = mouse_offset_y * y_offset * 0.05;

    renderer->zoomFactor += y_offset * 0.05 * renderer->zoomFactor;

    renderer->xOffset -= zoom_offset_diff_x;
    renderer->yOffset -= zoom_offset_diff_y;

    renderer->stateChanged = true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    window_width = width;
    window_height = height;

    userPointer* ptr = glfwGetWindowUserPointer(window);
    Renderer* renderer = ptr->renderer;

    renderer->stateChanged = true;
}

void mouse_position_callback(GLFWwindow* window, double x_pos, double y_pos) {
    int mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    userPointer* ptr = glfwGetWindowUserPointer(window);

    if (mouse_state == GLFW_PRESS) {
        Renderer* renderer = ptr->renderer;
        renderer->xOffset += x_pos - ptr->mouseX;
        renderer->yOffset -= y_pos - ptr->mouseY;

        renderer->stateChanged = true;
    }
    ptr->mouseX = x_pos;
    ptr->mouseY = y_pos;
}

void init_input(Renderer* renderer) {
    glfwSetWindowUserPointer(renderer->window, &ptr);
    ptr.renderer = renderer;
    double mouse_x, mouse_y;
    glfwGetCursorPos(renderer->window, &mouse_x, &mouse_y);
    ptr.mouseX = mouse_x;
    ptr.mouseY = mouse_y;
    glfwSetCursorPosCallback(renderer->window, mouse_position_callback);
    glfwSetScrollCallback(renderer->window, scroll_callback);
    glfwSetFramebufferSizeCallback(renderer->window, framebuffer_size_callback);
}

