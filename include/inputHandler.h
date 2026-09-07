#pragma once
#include <renderer.h>

typedef struct {
    Renderer* renderer;
    float mouse_x;
    float mouse_y;
} user_pointer;

void init_input(Renderer* renderer);
void update_coords(Renderer* renderer);
