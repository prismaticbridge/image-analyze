#pragma once
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    GLFWwindow* window;
    uint32_t textureId;
    uint32_t framebufferId;
    float zoomFactor;
    float xOffset, yOffset;
    uint32_t framebufferWidth, framebufferHeight;
    //stored bounding boxes
    uint32_t screenCoords[4];
    uint32_t fbCoords[4];
    bool stateChanged;
} Renderer;
Renderer init_renderer(const char* path);
void render(Renderer* renderer);
