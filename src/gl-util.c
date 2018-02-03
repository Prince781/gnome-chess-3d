#include <stdio.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include <gio/gio.h>
#include <errno.h>

#include "gl-util.h"

static FILE *
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

struct Obj3D *
load_OBJ(const char  *filename,
         GError     **error)
{
  FILE *file = NULL;
  GArray *temp_verts = g_array_new (false, false, sizeof (vec3_t));
  GArray *temp_uvs = g_array_new (false, false, sizeof (vec3_t));
  GArray *temp_normals = g_array_new (false, false, sizeof (vec3_t));

  GArray *tris = g_array_new (false, false, sizeof (int[3][3]));

  char *name = NULL;
  struct Obj3D *obj = NULL;

  if ((file = fopen_gresource (filename, error)) == NULL) {
    goto end;
  }

  char *line = NULL;
  size_t length = 0;

  while (getline(&line, &length, file) != -1) {
    char *space = strchr (line, ' ');

    if (!space)
      continue;

    if (strncmp(line, "v", space - line) == 0) {
      vec3_t vertex;

      sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      g_array_append_val (temp_verts, vertex);
    } else if (strncmp(line, "vt", space - line) == 0) {
      vec3_t texcoord;

      sscanf(line, "vt %f %f", &texcoord.x, &texcoord.y);
      g_array_append_val (temp_uvs, texcoord);
    } else if (strncmp(line, "vn", space - line) == 0) {
      vec3_t normal;

      sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
      g_array_append_val (temp_normals, normal);
    } else if (strncmp(line, "f", space - line) == 0) {
      int face[3][3];

      if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
             &face[0][0], &face[0][1], &face[0][2],
             &face[1][0], &face[1][1], &face[1][2],
             &face[2][0], &face[2][1], &face[2][2]) != 9) {
        if (sscanf(line, "f %d//%d %d//%d %d//%d",
               &face[0][0], &face[0][2],
               &face[1][0], &face[1][2],
               &face[2][0], &face[2][2]) == 6) {
          face[0][1] = -1;
          face[1][1] = -1;
          face[2][1] = -1;
          g_array_append_val (tris, face);
        }
      } else {
        g_array_append_val (tris, face);
      }
    } else if (strncmp(line, "o", space - line) == 0) {
      sscanf(line, "o %ms", &name);
    } else {
      line [strlen (line) - 1] = '\0';
      g_debug ("OBJ loader: skipping line '%s'", line);
    }
  }

  if (!feof(file)) {
    *error = g_error_new (g_quark_from_static_string ("OBJ loader"),
                          errno, "%s", strerror (errno));
    goto end;
  }

  /* Create the object.
   * 1. for all faces f in F:
   *  1a. for all points p in f: (vert_idx,uv_idx,normal_idx) = p
   *  1b. push new vec[3 + 2 + 3] into array of vertices
   * 2. convert
   */
  obj = calloc (1, sizeof(*obj));

  obj->name = name;
  obj->verts_size = tris->len * 3;
  obj->verts = calloc (obj->verts_size, sizeof (*obj->verts) * 8);

  for (int i=0; i<tris->len; ++i) {
    int triangle[3][3];

    memcpy(triangle, ((int (*)[3][3])(tris->data))[i], sizeof triangle);

    for (int p=0; p<3; ++p) {
      memcpy(&obj->verts[i*8*3 + 8*p], &temp_verts->data[triangle[p][0]], sizeof(vec3_t));
      if (triangle[p][1] >= 0)
        memcpy(&obj->verts[i*8*3 + 8*p + 3], &temp_verts->data[triangle[p][1]], sizeof(GLfloat[2]));
      memcpy(&obj->verts[i*8*3 + 8*p + 5], &temp_verts->data[triangle[p][2]], sizeof(vec3_t));
    }
  }

end:
  g_array_unref (temp_verts);
  g_array_unref (temp_uvs);
  g_array_unref (temp_normals);

  g_array_unref (tris);
  if (file)
    fclose(file);

  return obj;
}
