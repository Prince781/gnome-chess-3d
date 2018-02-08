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
                       "name", "Camera",
                       "fov", fov,
                       "near", near,
                       "far", far,
                       NULL);
}

static void
chess3d_camera_finalize (GObject *object)
{
  Chess3dCamera *self = (Chess3dCamera *)object;
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);

  G_OBJECT_CLASS (chess3d_camera_parent_class)->finalize (object);
}

void chess3d_camera_set_fov (Chess3dCamera *self,
                             float          fov)
{
  GValue value = G_VALUE_INIT;
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, fov);
  g_object_set_property (G_OBJECT (self), "fov", &value);
}

float chess3d_camera_get_fov (Chess3dCamera *self) {
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->fov;
}

void chess3d_camera_set_near (Chess3dCamera *self,
                              float          near)
{
  GValue value = G_VALUE_INIT;
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, near);
  g_object_set_property (G_OBJECT (self), "near", &value);
}

float chess3d_camera_get_near (Chess3dCamera *self)
{
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->near;
}

void chess3d_camera_set_far (Chess3dCamera *self,
                             float          far)
{
  GValue value = G_VALUE_INIT;
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, far);
  g_object_set_property (G_OBJECT (self), "far", &value);
}

float chess3d_camera_get_far (Chess3dCamera *self)
{
  Chess3dCameraPrivate *priv = chess3d_camera_get_instance_private (self);
  return priv->far;
}

static void
chess3d_camera_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  Chess3dCamera *self = CHESS3D_CAMERA (object);

  switch (prop_id)
    {
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
