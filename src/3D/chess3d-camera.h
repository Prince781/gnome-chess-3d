#ifndef CHESS3D_CAMERA_H
#define CHESS3D_CAMERA_H

#include <glib-object.h>
#include "chess3d-game-object.h"

G_BEGIN_DECLS

#define CHESS3D_TYPE_CAMERA (chess3d_camera_get_type())

G_DECLARE_DERIVABLE_TYPE (Chess3dCamera, chess3d_camera, CHESS3D, CAMERA, Chess3dGameObject)

struct _Chess3dCameraClass
{
  Chess3dGameObjectClass parent;
};

/**
 * chess3d_camera_new:
 *
 * Creates a new camera.
 *
 * @fov: field of view
 * @near: distance to near plane
 * @far: distance to far plane
 * Returns: a new Chess3dCamera
 */
Chess3dCamera *chess3d_camera_new (float fov, float near, float far);

void chess3d_camera_set_fov (Chess3dCamera *self,
                             float          fov);

float chess3d_camera_get_fov (Chess3dCamera *self);

void chess3d_camera_set_near (Chess3dCamera *self,
                              float          near);

float chess3d_camera_get_near (Chess3dCamera *self);

void chess3d_camera_set_far (Chess3dCamera *self,
                             float          far);

float chess3d_camera_get_far (Chess3dCamera *self);

mat4_t chess3d_camera_get_view (Chess3dCamera *self);

mat4_t chesss3d_camera_get_projection (Chess3dCamera *self,
                                       float          aspect_ratio);

G_END_DECLS

#endif /* CHESS3D_CAMERA_H */

