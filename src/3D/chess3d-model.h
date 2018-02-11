#pragma once

#include <glib-object.h>
#include "wavefront-object.h"
#include "vec3.h"

G_BEGIN_DECLS

#define CHESS3D_TYPE_MODEL (chess3d_model_get_type())

G_DECLARE_DERIVABLE_TYPE (Chess3dModel, chess3d_model, CHESS3D, MODEL, GInitiallyUnowned)

struct _Chess3dModelClass
{
  GObjectClass parent;
};

Chess3dModel *chess3d_model_new (WavefrontObject *object);

WavefrontObject *
chess3d_model_get_object (Chess3dModel *self);

void chess3d_model_set_color (Chess3dModel *self,
                              vec3_t        color);

vec3_t chess3d_model_get_color (Chess3dModel *self);

G_END_DECLS
