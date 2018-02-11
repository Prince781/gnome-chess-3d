#include "chess3d-game-object.h"
#include "vec3.h"
#include "mat4.h"

typedef struct
{
  GString *name;
  vec3_t position;
  vec3_t rotation;
  Chess3dModel *model;
} Chess3dGameObjectPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Chess3dGameObject, chess3d_game_object, G_TYPE_INITIALLY_UNOWNED)

enum {
  PROP_0,
  PROP_NAME,
  PROP_POSITION,
  PROP_ROTATION,
  PROP_MODEL,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

Chess3dGameObject *
chess3d_game_object_new (const char *name)
{
  return g_object_new (CHESS3D_TYPE_GAME_OBJECT,
                       "name", name,
                       NULL);
}

static void
chess3d_game_object_finalize (GObject *object)
{
  Chess3dGameObject *self = (Chess3dGameObject *)object;
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  g_string_free (priv->name, TRUE);
  G_OBJECT_CLASS (chess3d_game_object_parent_class)->finalize (object);
}

static void
chess3d_game_object_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  Chess3dGameObject *self = CHESS3D_GAME_OBJECT (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, chess3d_game_object_get_name (self));
      break;
    case PROP_POSITION: {
      vec3_t pos = chess3d_game_object_get_position (self);
      g_value_set_boxed (value, &pos);
    } break;
    case PROP_ROTATION: {
      vec3_t rot = chess3d_game_object_get_rotation (self);
      g_value_set_boxed (value, &rot);
    } break;
    case PROP_MODEL:
      g_value_set_object (value, chess3d_game_object_get_model (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_game_object_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  Chess3dGameObject *self = CHESS3D_GAME_OBJECT (object);
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_NAME:
      g_string_assign (priv->name, g_value_get_string (value));
      break;
    case PROP_POSITION:
      chess3d_game_object_set_position (self, *(vec3_t *)g_value_get_boxed (value));
      break;
    case PROP_ROTATION:
      chess3d_game_object_set_rotation (self, *(vec3_t *)g_value_get_boxed (value));
      break;
    case PROP_MODEL:
      chess3d_game_object_set_model (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
chess3d_game_object_class_init (Chess3dGameObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = chess3d_game_object_finalize;
  object_class->get_property = chess3d_game_object_get_property;
  object_class->set_property = chess3d_game_object_set_property;

  properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The name of the game object.",
                         "Untitled",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_POSITION] =
    g_param_spec_boxed ("position",
                        "Position",
                        "The position of the game object.",
                        VEC3_TYPE,
                        G_PARAM_READWRITE);

  properties[PROP_ROTATION] =
    g_param_spec_boxed ("rotation",
                        "Rotation",
                        "The rotation of the game object.",
                        VEC3_TYPE,
                        G_PARAM_READWRITE);

  properties [PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "Model",
                         CHESS3D_TYPE_MODEL,
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
chess3d_game_object_init (Chess3dGameObject *self)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  priv->name = g_string_new ("Untitled");
  priv->position = vec3 (0, 0, 0);
  priv->rotation = vec3 (0, 0, 0);
}

const gchar *
chess3d_game_object_get_name (Chess3dGameObject *self)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  return priv->name->str;
}

vec3_t
chess3d_game_object_get_position (Chess3dGameObject *self)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  return priv->position;
}

void
chess3d_game_object_set_position (Chess3dGameObject *self,
                                  vec3_t             position)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  priv->position = position;
}

vec3_t
chess3d_game_object_get_rotation (Chess3dGameObject *self)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  return priv->rotation;
}

void
chess3d_game_object_set_rotation (Chess3dGameObject *self,
                                  vec3_t             rotation)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  priv->rotation = rotation;
}

Chess3dModel *
chess3d_game_object_get_model (Chess3dGameObject *self)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  return priv->model;
}

void
chess3d_game_object_set_model (Chess3dGameObject *self,
                               Chess3dModel      *model)
{
  Chess3dGameObjectPrivate *priv = chess3d_game_object_get_instance_private (self);

  priv->model = g_object_ref_sink (model);
}
