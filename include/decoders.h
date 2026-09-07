#pragma once
#include <glad/gl.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
  IMAGE_UNKNOWN,
  IMAGE_PNG,
  IMAGE_JPEG,
  IMAGE_BMP,
  IMAGE_HEIC,
} ImageType;

typedef struct {
  char *output_buffer;
  size_t output_buffer_size;
  ImageType type;
  atomic_uint_fast32_t width;
  atomic_uint_fast32_t height;
  GLenum gl_format;
} OutputBuffer;

typedef struct {
  const char *path;
  OutputBuffer *out;
} LoadArgs;

void *load_image_thread(void *arg_struct);
