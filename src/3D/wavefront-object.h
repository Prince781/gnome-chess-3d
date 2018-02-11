#pragma once

#include <glib-object.h>
#include <epoxy/gl.h>
#include <glib.h>

G_BEGIN_DECLS

#define WAVEFRONT_TYPE_OBJECT (wavefront_object_get_type())

G_DECLARE_DERIVABLE_TYPE (WavefrontObject, wavefront_object, WAVEFRONT, OBJECT, GInitiallyUnowned)

struct _WavefrontObjectClass
{
  GObjectClass parent;
};

/**
 * wavefront_object_new:
 *
 * Loads an object from a location.
 *
 * Returns: a new Wavefront Object
 */
WavefrontObject *wavefront_object_new (const char  *location,
                                       GError     **error);

/**
 * wavefront_object_get_name:
 *
 * The name of the object, specified in the file.
 *
 * Returns: the name of the object
 */
const gchar *
wavefront_object_get_name (WavefrontObject *self);

/**
 * wavefront_object_get_location:
 *
 * Gets the location of the resource that this object came from.
 *
 * Returns: a path
 */
const gchar *
wavefront_object_get_location (WavefrontObject *self);

/**
 * wavefront_object_generate_buffers:
 *
 * Generates a VAO and VBO for this object.
 *
 * Returns:
 */
void wavefront_object_generate_buffers (WavefrontObject *self);

/**
 * wavefront_object_hook_shader_attributes:
 *
 * Hooks up shader attributes to the VBO. If @wavefront_object_generate_buffers
 * was not called previously, then the VAO and VBO are generated.
 *
 * Returns:
 */
void wavefront_object_hook_shader_attributes (WavefrontObject *self,
                                              GLuint           program);

void wavefront_object_render (WavefrontObject *self);

GLuint wavefront_object_get_vao (WavefrontObject *self);

GLuint wavefront_object_get_vbo (WavefrontObject *self);

G_END_DECLS
