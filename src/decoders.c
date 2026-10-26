#include <decoders.h>
#include <spng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

#ifdef HAVE_LIBHEIF
#include <libheif/heif.h>
#endif

const static char *const png_bytes = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
const static char *const jpeg_bytes = "\xFF\xD8\xFF";
const static char *const heif_header_bytes =
    "\x00\x00\x00\x18\x66\x74\x79\x70\x68\x65\x69\x63\x00\x00\x00\x00";

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

void print_gl_format_info(GLenum gl_format) {
  switch (gl_format) {
  case GL_RGB8:
    printf("Detected RGB (no alpha channel), assuming 8 bit depth\n");
    break;
  case GL_RGBA8:
    printf("Detected RGBA with 8 bit depth\n");
    break;
  case GL_RGBA16:
    printf("Detected RGBA with >8 bit depth, using 16 bits");
    break;
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
  fclose(fp);

  out->gl_format = spng_to_gl_format(format);
  out->type = IMAGE_PNG;
  print_gl_format_info(out->gl_format);
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
                (unsigned char *)out->output_buffer, width, 0, height, TJPF_RGB,
                0);

  out->gl_format = GL_RGB8;
  print_gl_format_info(out->gl_format);
  out->type = IMAGE_JPEG;
  tjDestroy(tj);
  free(jpeg_buf);
  return out;
}

#ifdef HAVE_LIBHEIF
void *decode_heif(FILE *fp, OutputBuffer *out) {
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  unsigned char *buf = malloc(size);
  fread(buf, 1, size, fp);
  fclose(fp);

  heif_context *ctx = heif_context_alloc();
  heif_context_read_from_memory_without_copy(ctx, buf, size, NULL);

  heif_image_handle *handle = NULL;
  heif_error err = heif_context_get_primary_image_handle(ctx, &handle);
  if (err.code != heif_error_Ok || handle == NULL) {
    heif_context_free(ctx);
    free(buf);
    return NULL;
  }

  int width = heif_image_handle_get_width(handle);
  int height = heif_image_handle_get_height(handle);
  out->width = width;
  out->height = height;

  int has_alpha = heif_image_handle_has_alpha_channel(handle);
  int channels = has_alpha ? 4 : 3;
  enum heif_chroma chroma =
      has_alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB;

  heif_image *image = NULL;
  err = heif_decode_image(handle, &image, heif_colorspace_RGB, chroma, NULL);
  if (err.code != heif_error_Ok || image == NULL) {
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    free(buf);
    return NULL;
  }

  int stride = 0;
  const uint8_t *data =
      heif_image_get_plane_readonly(image, heif_channel_interleaved, &stride);

  const size_t output_size = (size_t)width * height * channels;

  if (stride == width * channels) {
    // memcpy(out->output_buffer, data, output_size);
    out->output_buffer = (char *)data;
    out->output_buffer_size = output_size;
  } else {
    if (out->output_buffer == NULL) {
      out->output_buffer = malloc(output_size);
      out->output_buffer_size = output_size;
    } else if (out->output_buffer_size < output_size) {
      out->output_buffer = realloc(out->output_buffer, output_size);
      out->output_buffer_size = output_size;
    }
    for (int y = 0; y < height; y++) {
      memcpy((char *)out->output_buffer + (size_t)y * width * channels,
             data + (size_t)y * stride, (size_t)width * channels);
    }
  }

  GLenum gl_format;
  if (has_alpha) {
    int bit_depth =
        heif_image_get_bits_per_pixel_range(image, heif_channel_interleaved);
    if (bit_depth > 8) {
      gl_format = GL_RGBA16;
    } else {
      gl_format = GL_RGBA8;
    }
  } else {
    gl_format = GL_RGB8;
  }
  print_gl_format_info(gl_format);
  out->gl_format = gl_format;
  out->type = IMAGE_HEIC;

  // heif_image_release(image);
  heif_image_handle_release(handle);
  heif_context_free(ctx);
  free(buf);
  return out;
}
#endif

void *load_image_thread(void *arg_ptr) {
#ifdef HAVE_LIBHEIF
  printf("libheif support enabled (v%s)\n", heif_get_version());
#endif
  LoadArgs *args = arg_ptr;
  FILE *fp = fopen(args->path, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", (char *)args->path);
    return NULL;
  }
  unsigned char header[32];

  size_t bytes_read = fread(header, 1, sizeof(header), fp);
  rewind(fp);

  ImageType type = IMAGE_UNKNOWN;
  if (bytes_read >= 8 &&
      !memcmp((void *)png_bytes, (void *)header, (size_t)8)) {
    type = IMAGE_PNG;
    printf("Detected png\n");
  } else if (bytes_read >= 3 &&
             !memcmp((void *)jpeg_bytes, (void *)header, (size_t)3)) {
    type = IMAGE_JPEG;
    printf("Detected jpeg\n");
  } else if ((bytes_read >= 16 && !memcmp(header, heif_header_bytes, 16)) ||
             (bytes_read >= 12 && (!memcmp(header + 4, "ftypheic", 8) ||
                                   !memcmp(header + 4, "ftypm", 5))) ||
             (bytes_read >= 8 && (!memcmp(header, "ftypheic", 8) ||
                                  !memcmp(header, "ftypm", 5)))) {
    type = IMAGE_HEIC;
    printf("Detected heif/heic\n");
  }

  switch (type) {
  case IMAGE_PNG:
    return decode_spng(fp, args->out);
  case IMAGE_JPEG:
    return decode_jpeg(fp, args->out);
  case IMAGE_HEIC:
#ifdef HAVE_LIBHEIF
    return decode_heif(fp, args->out);
#else
    fclose(fp);
    fprintf(stderr, "Error: To decode HEIF/HEIC images, please install libheif "
                    "and then rebuild/reinstall this package.\n");
    return NULL;
#endif
  default:
    fclose(fp);
    return NULL;
  }
}
