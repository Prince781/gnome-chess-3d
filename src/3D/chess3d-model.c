#include "chess3d-model.h"
#include "../gl-util.h"

typedef struct
{
  struct Obj3D *model;
} Chess3dModelPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Chess3dModel, chess3d_model, CHESS3D_TYPE_GAME_OBJECT)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

Chess3dModel *
chess3d_model_new (const char *name)
{
  return g_object_new (CHESS3D_TYPE_MODEL,
                       "name", name,
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

  switch (prop_id)
    {
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
}

static void
chess3d_model_init (Chess3dModel *self)
{
}
