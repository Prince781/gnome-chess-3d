#include "chess3d-model.h"
#include "wavefront-object.h"
#include "vec3.h"
#include "../gl-util.h"
#include <epoxy/gl.h>

typedef struct
{
  WavefrontObject *object;
  vec3_t color;
} Chess3dModelPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Chess3dModel, chess3d_model, G_TYPE_INITIALLY_UNOWNED)

enum {
  PROP_0,
  PROP_OBJECT,
  PROP_COLOR,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

Chess3dModel *
chess3d_model_new (WavefrontObject *object)
{
  g_return_val_if_fail (object != NULL, NULL);

  return g_object_new (CHESS3D_TYPE_MODEL,
                       "object", object,
                       NULL);
}

static void
chess3d_model_finalize (GObject *object)
{
  Chess3dModel *self = (Chess3dModel *)object;
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  G_OBJECT_CLASS (chess3d_model_parent_class)->finalize (object);
}

static void
chess3d_model_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  Chess3dModel *self = CHESS3D_MODEL (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, chess3d_model_get_object (self));
      break;
    case PROP_COLOR: {
      vec3_t color = chess3d_model_get_color (self);
      g_value_set_boxed (value, &color);
    } break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_model_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  Chess3dModel *self = CHESS3D_MODEL (object);
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_OBJECT:
      priv->object = g_value_get_object (value);
      break;
    case PROP_COLOR: {
      chess3d_model_set_color (self, *(vec3_t *) g_value_get_boxed (value));
    } break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_model_class_init (Chess3dModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = chess3d_model_finalize;
  object_class->get_property = chess3d_model_get_property;
  object_class->set_property = chess3d_model_set_property;

  properties [PROP_OBJECT] =
    g_param_spec_object ("object",
                         "Object",
                         "The Wavefront object that is loaded",
                         WAVEFRONT_TYPE_OBJECT,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));

  properties [PROP_COLOR] =
    g_param_spec_boxed ("color",
                        "Color",
                        "Color",
                        VEC3_TYPE,
                        (G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
chess3d_model_init (Chess3dModel *self)
{
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  priv->color = vec3(1, 1, 1);
}

WavefrontObject *
chess3d_model_get_object (Chess3dModel *self)
{
  g_return_val_if_fail (self, NULL);
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  return priv->object;
}

void chess3d_model_set_color (Chess3dModel *self,
                              vec3_t        color)
{
  g_return_if_fail (self);
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  priv->color = vec3(CLAMP (color.x, 0, 1), CLAMP (color.y, 0, 1), CLAMP (color.z, 0, 1));
}

vec3_t chess3d_model_get_color (Chess3dModel *self)
{
  g_return_val_if_fail (self, vec3 (0, 0, 0));
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  return priv->color;
}

void chess3d_model_render (Chess3dModel *self,
                           GLuint        program)
{
  g_return_if_fail (self);
  Chess3dModelPrivate *priv = chess3d_model_get_instance_private (self);

  shader_program_set_vec3 (program, "overrideColor", priv->color);
  wavefront_object_render (priv->object);
}
