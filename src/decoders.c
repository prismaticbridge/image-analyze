#include <decoders.h>
#include <spng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

const static char *const png_bytes = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
const static char *const jpeg_bytes = "\xFF\xD8\xFF";

GLenum spng_to_gl_format(uint8_t spng_format) {
  switch (spng_format) {
  case SPNG_FMT_RGB8:
    return GL_RGB8;
  case SPNG_FMT_RGBA8:
    return GL_RGBA8;
  case SPNG_FMT_RGBA16:
    return GL_RGBA16;
  default:
    return 0;
  }
}

void *decode_spng(FILE *fp, OutputBuffer *out) {
  spng_ctx *ctx = spng_ctx_new(0);
  spng_set_png_file(ctx, fp);

  struct spng_ihdr ihdr;
  spng_get_ihdr(ctx, &ihdr);
  out->width = ihdr.width;
  out->height = ihdr.height;
  uint8_t format;
  switch (ihdr.color_type) {
  case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
    format = (ihdr.bit_depth > 8) ? SPNG_FMT_RGBA16 : SPNG_FMT_RGBA8;
    break;
  default:
    format = SPNG_FMT_RGB8;
  }
  size_t output_size;
  spng_decoded_image_size(ctx, format, &output_size);

  if (out->output_buffer == NULL) {
    out->output_buffer = malloc(output_size);
    out->output_buffer_size = output_size;
  } else if (out->output_buffer_size < output_size) {
    out->output_buffer = realloc(out->output_buffer, output_size);
    out->output_buffer_size = output_size;
  }

  spng_decode_image(ctx, out->output_buffer, output_size, format, 0);

  out->gl_format = spng_to_gl_format(format);
  spng_ctx_free(ctx);
  return out;
}

void *decode_jpeg(FILE *fp, OutputBuffer *out) {
  fseek(fp, 0, SEEK_END);
  long jpeg_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  unsigned char *jpeg_buf = malloc(jpeg_size);
  fread(jpeg_buf, 1, jpeg_size, fp);
  fclose(fp);

  tjhandle tj = tjInitDecompress();
  int width, height, subsamp, colorspace;
  tjDecompressHeader3(tj, jpeg_buf, (unsigned long)jpeg_size, &width, &height,
                      &subsamp, &colorspace);

  out->width = width;
  out->height = height;

  size_t output_size = (size_t)width * height * 3;
  if (out->output_buffer == NULL) {
    out->output_buffer = malloc(output_size);
    out->output_buffer_size = output_size;
  } else if (out->output_buffer_size < output_size) {
    out->output_buffer = realloc(out->output_buffer, output_size);
    out->output_buffer_size = output_size;
  }

  tjDecompress2(tj, jpeg_buf, (unsigned long)jpeg_size,
                (unsigned char *)out->output_buffer, width, 0, height,
                TJPF_RGB, 0);

  out->gl_format = GL_RGB8;
  out->type = IMAGE_JPEG;
  tjDestroy(tj);
  free(jpeg_buf);
  return out;
}

void *load_image_thread(void *arg_ptr) {
  LoadArgs *args = arg_ptr;
  FILE *fp = fopen(args->path, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", (char *)args->path);
    return NULL;
  }
  unsigned char header[12];

  size_t bytes_read = fread(header, 1, sizeof(header), fp);
  rewind(fp);

  ImageType type = IMAGE_UNKNOWN;
  if (bytes_read >= 8 && !memcmp((void *)png_bytes, (void *)header, (size_t)8)) {
    type = IMAGE_PNG;
    printf("Detected png\n");
  } else if (bytes_read >= 3 &&
             !memcmp((void *)jpeg_bytes, (void *)header, (size_t)3)) {
    type = IMAGE_JPEG;
    printf("Detected jpeg\n");
  }

  switch (type) {
  case IMAGE_PNG:
    return decode_spng(fp, args->out);
  case IMAGE_JPEG:
    return decode_jpeg(fp, args->out);
  default:
    return NULL;
  }
}
