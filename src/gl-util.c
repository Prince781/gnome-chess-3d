#include <stdio.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include <gio/gio.h>
#include <errno.h>

#include "gl-util.h"

FILE *
fopen_gresource(const char  *filename,
                GError     **error)
{
    FILE *file = NULL;
    GBytes *bytes = NULL;
    size_t size;
    void *data;

    if ((bytes = g_resources_lookup_data(filename,
                                         G_RESOURCE_LOOKUP_FLAGS_NONE,
                                         error)) == NULL) {
        return NULL;
    }

    data = g_bytes_unref_to_data(bytes, &size);

    if ((file = fmemopen(data, size, "r")) == NULL) {
        *error = g_error_new(g_quark_from_static_string("Error"),
                    errno, "Could not open bytes: %s", strerror(errno));
        return NULL;
    }

    return file;
}

char **
get_lines(const char  *filename,
          int         *count,
          GError     **error)
{
  FILE *file;
  char *next_line = NULL;
  size_t length = 0;
  GPtrArray *lines;

  file = fopen_gresource(filename, error);

  if (*error)
    return NULL;

  lines = g_ptr_array_new ();
  while (getline(&next_line, &length, file) != -1) {
    g_ptr_array_add (lines, g_strdup (next_line));
  }

  *count = lines->len;
  g_debug ("Read %d lines from %s", lines->len, filename);

  fclose(file);
  return (char **) g_ptr_array_free (lines, false);
}

unsigned char *
load_image(const char  *filename,
           int         *width,
           int         *height,
           GLenum      *format,
           GError     **error)
{
  GdkPixbuf *pixbuf = NULL;
  guchar *data = NULL;
  GdkPixbufFormat *file_info = NULL;

  if ((pixbuf = gdk_pixbuf_new_from_resource (filename, error)) == NULL) {
    return NULL;
  }

  switch (gdk_pixbuf_get_n_channels(pixbuf)) {
  case 1:
    *format = GL_LUMINANCE;
    break;
  case 3:
    *format = GL_RGB;
    break;
  case 4:
    *format = GL_RGBA;
    break;
  default:
    *error = g_error_new (g_quark_from_static_string ("Texture Load"), 0,
                          "Image %s doesn't have 1,3, or 4 channels.",
                          filename);
    goto end;
  }

  file_info = gdk_pixbuf_get_file_info(filename, width, height);
  if (file_info == NULL) {
    *error = g_error_new (g_quark_from_static_string ("Texture Load"), 0,
                          "Unknown file format for %s",
                          filename);
  } else {
    g_debug ("Loaded %s (%s) (%dx%d)",
             gdk_pixbuf_format_get_name (file_info),
             gdk_pixbuf_format_get_description (file_info),
             *width, *height);
    /* if the file is not read-only, then we have
     * to copy the pixel data manually, since
     * gdk_pixbuf_get_pixels() won't return a copy. */
    if (gdk_pixbuf_format_is_writable(file_info)) {
      guint length;
      guchar *data_ref;
      data_ref = gdk_pixbuf_get_pixels_with_length(pixbuf, &length);
      data = g_memdup (data_ref, length);
    } else
        data = gdk_pixbuf_get_pixels(pixbuf);
  }

end:
  g_object_unref(pixbuf);
  return data;
}

void
texture_load(GLuint       texture,
             const char  *filename,
             GError     **error)
{
  unsigned char *image;
  int width, height;
  GLenum img_format;

  // texture is bound to a texture unit previously
  glBindTexture(GL_TEXTURE_2D, texture);
  image = load_image(filename, &width, &height, &img_format, error);
  if (image != NULL) {
    /* load an image:
     * param[1] = level of detail (for custom mipmap images)
     * param[2] = *internal* pixel format (RGB = 3 colors)
     * param[3,4] = width and height of image in pixels
     * param[5] = must be 0
     * param[6] = format of the pixels in the given array
     * param[7] = data type of the given array
     * param[8] = the array */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, img_format,
            GL_UNSIGNED_BYTE, image);
    g_debug ("Loaded texture from %s", filename);
    free(image);
  }
}

GLuint
shader_new (const char  *filename,
            GLuint       type,
            GError     **error)
{
  GLuint shader;
  int num_lines;
  char **lines;

  if ((lines = get_lines(filename, &num_lines, error)) == NULL)
      return -1;

  shader = glCreateShader (type);
  glShaderSource (shader, num_lines, (const GLchar **) lines, NULL);

  g_debug ("Loading shader from %s", filename);

  free (lines);
  return shader;
}

void
shader_compile (GLuint   shader,
                GError **error)
{
  GLint status;

  glCompileShader (shader);
  glGetShaderiv (shader, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE) {
    GLchar *buf;
    GLint length;

    glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &length);
    buf = g_malloc0 (length);
    glGetShaderInfoLog (shader, length, NULL, buf);
    *error = g_error_new (g_quark_from_static_string ("Shader Compile"),
                          0, "%s", buf);
  }
}

void
shader_program_link (GLuint   program,
                     GError **error)
{
  GLint status;

  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status == GL_FALSE) {
    char *buf;
    int length;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    buf = g_malloc0 (length);
    glGetProgramInfoLog(program, length, NULL, buf);
    *error = g_error_new (g_quark_from_static_string ("Shader Program Link"),
                          0, "%s", buf);
  }
}
