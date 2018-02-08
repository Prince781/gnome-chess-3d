#ifndef CHESS3D_MODEL_H
#define CHESS3D_MODEL_H

#include <glib-object.h>
#include "chess3d-game-object.h"

G_BEGIN_DECLS

#define CHESS3D_TYPE_MODEL (chess3d_model_get_type())

G_DECLARE_DERIVABLE_TYPE (Chess3dModel, chess3d_model, CHESS3D, MODEL, Chess3dGameObject)

struct _Chess3dModelClass
{
  Chess3dGameObjectClass parent;
};

/**
 * chess3d_model_new:
 *
 * Creates a new model. A model has vertex data.
 *
 * Returns: a new Model
 */
Chess3dModel *chess3d_model_new (const char *name);

G_END_DECLS

#endif /* CHESS3D_MODEL_H */

