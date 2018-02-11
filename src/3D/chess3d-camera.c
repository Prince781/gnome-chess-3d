#include "chess3d-camera.h"

typedef struct
{
  float fov;
  float near;
  float far;
} Chess3dCameraPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Chess3dCamera, chess3d_camera, CHESS3D_TYPE_GAME_OBJECT)

enum {
  PROP_0,
  PROP_FOV,
  PROP_NEAR,
  PROP_FAR,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

Chess3dCamera *
chess3d_camera_new (float fov, float near, float far)
{
  return g_object_new (CHESS3D_TYPE_CAMERA,
                       "fov", fov,
                       "near", near,
                       "far", far,
                       "name", "Camera",
                       NULL);
}

static void
chess3d_camera_finalize (GObject *object)
{
  Chess3dCamera *self = (Chess3dCamera *)object;
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);

  G_OBJECT_CLASS (chess3d_camera_parent_class)->finalize (object);
}

static void
chess3d_camera_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  Chess3dCamera *self = CHESS3D_CAMERA (object);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_FOV:
      g_value_set_float (value, priv->fov);
      break;
    case PROP_NEAR:
      g_value_set_float (value, priv->near);
      break;
    case PROP_FAR:
      g_value_set_float (value, priv->far);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_camera_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  Chess3dCamera *self = CHESS3D_CAMERA (object);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_FOV:
      priv->fov = g_value_get_float (value);
      break;
    case PROP_NEAR:
      priv->near = g_value_get_float (value);
      break;
    case PROP_FAR:
      priv->far = g_value_get_float (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_camera_class_init (Chess3dCameraClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = chess3d_camera_finalize;
  object_class->get_property = chess3d_camera_get_property;
  object_class->set_property = chess3d_camera_set_property;

  properties[PROP_FOV] =
    g_param_spec_float ("fov",
                        "Field of View",
                        "The field of view, in degrees.",
                        1.f, 360.f, 60.f,
                        G_PARAM_READWRITE);

  properties[PROP_NEAR] =
    g_param_spec_float ("near",
                        "Near",
                        "The distance to the near plane",
                        0.1f, 10.f, 1.f,
                        G_PARAM_READWRITE);

  properties[PROP_FAR] =
    g_param_spec_float ("far",
                        "Far",
                        "The distance to the far plane",
                        10.f, 1000.f, 500.f,
                        G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
chess3d_camera_init (Chess3dCamera *self)
{
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);

  priv->fov = 60.f;
  priv->near = 1.f;
  priv->far = 1000.f;
}

void chess3d_camera_set_fov (Chess3dCamera *self,
                             float          fov)
{
  g_warn_if_fail (self);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  priv->fov = fov;
}

float chess3d_camera_get_fov (Chess3dCamera *self) {
  g_return_val_if_fail (self, 0);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->fov;
}

void chess3d_camera_set_near (Chess3dCamera *self,
                              float          near)
{
  g_warn_if_fail (self);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  priv->near = near;
}

float chess3d_camera_get_near (Chess3dCamera *self)
{
  g_return_val_if_fail (self, 0);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->near;
}

void chess3d_camera_set_far (Chess3dCamera *self,
                             float          far)
{
  g_warn_if_fail (self);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  priv->far = far;
}

float chess3d_camera_get_far (Chess3dCamera *self)
{
  g_return_val_if_fail (self, 0);
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->far;
}

mat4_t chess3d_camera_get_view (Chess3dCamera *self)
{
  g_return_val_if_fail (self, (mat4_t){});
  Chess3dGameObject *game_object = CHESS3D_GAME_OBJECT (self);
  vec3_t position = chess3d_game_object_get_position (game_object);
  vec3_t rotation = chess3d_game_object_get_rotation (game_object);
  vec3_t up = vec3(0, 1, 0);
  mat4_t rot_x = m4_rotation_x (rotation.x),
         rot_y = m4_rotation_y (rotation.y),
         rot_z = m4_rotation_z (rotation.z);
  vec3_t dir = m4_mul_dir (m4_mul (m4_mul (rot_x, rot_y), rot_z), up);

  /* view matrix:
   * arg1 = position of camera
   * arg2 = center of view
   * arg3 = direction up. Note that because UP is along the y-axis, this causes
   * (x,z)-plane to be perpendicular to "up"
   */
  return m4_look_at (position, dir, up);
}

mat4_t chesss3d_camera_get_projection (Chess3dCamera *self,
                                       float          aspect_ratio)
{
  g_return_val_if_fail (self, (mat4_t) {});
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return m4_perspective (priv->fov, aspect_ratio, priv->near, priv->far);
}
