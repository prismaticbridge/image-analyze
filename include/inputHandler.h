#pragma once
#include <renderer.h>

typedef struct {
    Renderer* renderer;
    float mouseX;
    float mouseY;
} userPointer;

void init_input(Renderer* renderer);
void update_coords(Renderer* renderer);
