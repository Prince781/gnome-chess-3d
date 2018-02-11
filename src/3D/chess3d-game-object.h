#ifndef CHESS3D_GAME_OBJECT_H
#define CHESS3D_GAME_OBJECT_H

#include <glib-object.h>
#include "vec3.h"
#include "chess3d-model.h"

G_BEGIN_DECLS

#define CHESS3D_TYPE_GAME_OBJECT (chess3d_game_object_get_type())

G_DECLARE_DERIVABLE_TYPE (Chess3dGameObject, chess3d_game_object, CHESS3D, GAME_OBJECT, GInitiallyUnowned)

struct _Chess3dGameObjectClass
{
  GObjectClass parent;
};

Chess3dGameObject *chess3d_game_object_new (const char *name);

const gchar *chess3d_game_object_get_name (Chess3dGameObject *self);

vec3_t chess3d_game_object_get_position (Chess3dGameObject *self);

void chess3d_game_object_set_position (Chess3dGameObject *self,
                                       vec3_t             position);

vec3_t chess3d_game_object_get_rotation (Chess3dGameObject *self);

void chess3d_game_object_set_rotation (Chess3dGameObject *self,
                                       vec3_t             rotation);

void chess3d_game_object_translate (Chess3dGameObject *self,
                                    vec3_t             translation);

void chess3d_game_object_rotate (Chess3dGameObject *self,
                                 vec3_t             rotation);

/**
 * chess3d_game_object_get_model_matrix:
 *
 * Gets the model matrix of the game object.
 *
 * Returns:
 */
mat4_t chess3d_game_object_get_model_matrix (Chess3dGameObject *self);

Chess3dModel *chess3d_game_object_get_model (Chess3dGameObject *self);

void chess3d_game_object_set_model (Chess3dGameObject *self,
                                    Chess3dModel      *model);

G_END_DECLS

#endif /* CHESS3D_GAME_OBJECT_H */

