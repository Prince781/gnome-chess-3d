#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <epoxy/gl.h>
#include "math_3d.h"
#include <glib.h>

/* Loads data from a file. (*count) is the number of
 * lines read.
 * Returns an array of lines
 */
char **
get_lines(const char  *filename,
          int         *count,
          GError     **error);

/* Loads image data from a file. Returns NULL otherwise.
 * (*format) = format of the data (GL_LUMINANCE, GL_RGB, or GL_RGBA).
 */
unsigned char *
load_image(const char  *filename,
           int         *width,
           int         *height,
           GLenum      *format,
           GError     **error);

void
texture_load(GLuint       texture,
             const char  *filename,
             GError     **error);

/* Creates a new shader and sets filename to be the source
 * of the shader. */
GLuint
shader_new(const char  *filename,
           GLuint       type,
           GError     **error);

/**
 * Compiles a new shader.
 */
void
shader_compile (GLuint   shader,
                GError **error);

/**
 * Links a GLSL program.
 */
void
shader_program_link (GLuint   program,
                     GError **error);

static inline void
shader_program_set_vec3(GLuint      program,
                        const char *name,
                        vec3_t      vec)
{
  glUniform3f (glGetUniformLocation (program, name),
               vec.x, vec.y, vec.z);
}

static inline void
shader_program_set_mat4(GLuint      program,
                        const char *name,
                        mat4_t      mat)
{
    glUniformMatrix4fv(glGetUniformLocation(program, name),
            1, GL_FALSE, &mat.m[0][0]);
}

struct Obj3D {
  char *name;

  /**
   * [vec3 | uv | normal ] = [8]
   */
  GLfloat *verts;

  /**
   * verts_size * 8 = length of array
   */
  int verts_size;

  /**
   * The VBO associated with the vertex data.
   */
  GLuint vbo;

  /**
   * The VAO associated with this object.
   */
  GLuint vao;
};

/**
 * loads a new wavefront .obj file.
 * Returns NULL on error.
 */
struct Obj3D *
load_OBJ(const char  *filename,
         GError     **error);

#endif
