#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <epoxy/gl.h>
#include "math_3d.h"
#include <glib.h>

/* Loads data from a file. (*count) is the number of
 * lines read.
 * Returns an array of lines
 */
char **get_lines(const char *filename, int *count, GError **error);

/* Loads image data from a file. Returns NULL otherwise.
 * (*format) = format of the data (GL_LUMINANCE, GL_RGB, or GL_RGBA).
 */
unsigned char *load_image(const char *filename,
        int *width, int *height, GLenum *format);

void texture_load(GLuint texture, const char *filename);

/* Creates a new shader and sets filename to be the source
 * of the shader. */
GLuint shader_new(const char *filename, GLuint type, GError **error);

static inline void shader_program_set_mat4(GLuint program,
        const char *name, mat4_t mat)
{
    glUniformMatrix4fv(glGetUniformLocation(program, name),
            1, GL_FALSE, &mat.m[0][0]);
}

#endif
