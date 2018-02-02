#include <stdio.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include <gio/gio.h>
#include <errno.h>

#include "gl-util.h"

static FILE *fopen_gresource(const char *filename, GError **error)
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

char **get_lines(const char *filename, int *count, GError **error)
{
    FILE *file;
    char **lines;
    char *next_line = NULL;
    size_t length = 0;
    int num_lines = 0;
    int capacity = 1;

    file = fopen_gresource(filename, error);

    g_return_val_if_fail(*error != NULL, NULL);

    lines = calloc(capacity, sizeof(*lines));
    while (getline(&next_line, &length, file) != -1) {
        if (num_lines >= capacity) {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(*lines));
        }
        lines[num_lines] = next_line;
        ++num_lines;
        next_line = NULL;
        length = 0;
    }

    printf("Read %d lines from %s\n", num_lines, filename);
    fclose(file);
    lines = realloc(lines, num_lines * sizeof(*lines));
    *count = num_lines;

    return lines;
}

unsigned char *load_image(const char *filename,
        int *width, int *height, GLenum *format)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    guchar *data = NULL;
    GdkPixbufFormat *file_info;

    if (pixbuf == NULL) {
        fprintf(stderr, "Unable to load texture: %s\n", error->message);
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
        fprintf(stderr, "%s: Unknown image format.\n", __func__);
        return NULL;
    }

    file_info = gdk_pixbuf_get_file_info(filename, width, height);
    if (file_info == NULL) {
        fprintf(stderr, "Unknown file format.\n");
    } else {
        printf("Loaded %s (%s) (%dx%d)\n",
                gdk_pixbuf_format_get_name(file_info),
                gdk_pixbuf_format_get_description(file_info),
                *width, *height);
        /* if the file is not read-only, then we have
         * to copy the pixel data manually, since
         * gdk_pixbuf_get_pixels() won't return a copy. */
        if (gdk_pixbuf_format_is_writable(file_info)) {
            guint length;
            guchar *data_ref;
            data_ref = gdk_pixbuf_get_pixels_with_length(pixbuf, &length);
            data = malloc(length * sizeof(*data));
            memcpy(data, data_ref, length*sizeof(*data_ref));
        } else
            data = gdk_pixbuf_get_pixels(pixbuf);
    }

    g_object_unref(pixbuf);
    return data;
}

void texture_load(GLuint texture, const char *filename)
{
    unsigned char *image;
    int width, height;
    GLenum img_format;

    // texture is bound to a texture unit previously
    glBindTexture(GL_TEXTURE_2D, texture);
    image = load_image(filename, &width, &height, &img_format);
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
        printf("Loaded texture %u.\n", texture);
        free(image);
    }
}

GLuint shader_new(const char *filename, GLuint type, GError **error)
{
    GLuint shader;
    int num_lines;
    char **lines;

    GLint status;
    char err_buf[512];

    shader = glCreateShader(type);
    if ((lines = get_lines(filename, &num_lines, error)) == NULL)
        return shader;
    glShaderSource(shader, num_lines, (const GLchar **) lines, NULL);

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glGetShaderInfoLog(shader, sizeof(err_buf), NULL, err_buf);
        fprintf(stderr, "Error compiling %s:\n%s\n", filename, err_buf);
    } else
        printf("Compiled shader %s\n", filename);

    free(lines);
    return shader;
}
