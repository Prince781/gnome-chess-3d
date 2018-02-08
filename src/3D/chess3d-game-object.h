#ifndef CHESS3D_GAME_OBJECT_H
#define CHESS3D_GAME_OBJECT_H

#include <glib-object.h>
#include "vec3.h"

G_BEGIN_DECLS

#define CHESS3D_TYPE_GAME_OBJECT (chess3d_game_object_get_type())

G_DECLARE_DERIVABLE_TYPE (Chess3dGameObject, chess3d_game_object, CHESS3D, GAME_OBJECT, GObject)

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

G_END_DECLS

#endif /* CHESS3D_GAME_OBJECT_H */

