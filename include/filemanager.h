#pragma once
#include <stdlib.h>
void expand_tilde(const char *input, char *output, size_t size);
int find_latest_screenshot(char *buffer, const char *directory);

/* Returns a sorted (alphabetical) list of image paths in the same directory as
 * filepath. The caller must free the result with free_image_files. */
char **list_image_files(const char *filepath, int *count);
void free_image_files(char **paths, int count);
