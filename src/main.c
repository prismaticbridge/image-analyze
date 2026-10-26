#include <filemanager.h>
#include <inputHandler.h>
#include <renderer.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  char *filepath;
  char buffer[256];
  if (argc == 2) {
    expand_tilde(argv[1], buffer, 256);
    filepath = buffer;
  } else if (argc == 1) {
    fgets(buffer, 256, stdin);
    if (buffer[0] == '\n') {
      char expanded[256];
      expand_tilde("~/Pictures/Screenshots", expanded, 256);
      int success = find_latest_screenshot(buffer, expanded);
      if (success != 0) {
        fprintf(stderr, "Failed to load %s\n", expanded);
        return 2;
      }
    }
    filepath = buffer;
  } else {
    fprintf(stderr, "Error: too many arguments\n");
    return 1;
  }

  Renderer renderer = init_renderer(filepath);
  init_input(&renderer);

  render(&renderer);
  while (!glfwWindowShouldClose(renderer.window)) {
    glfwWaitEvents();
    if (renderer.state_changed) {
      update_coords(&renderer);
      render(&renderer);
    }
  }

  free_image_files(renderer.image_paths, renderer.image_count);
  return 0;
}
