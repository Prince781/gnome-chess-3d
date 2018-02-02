#include "chess-view-3d.h"
#include <epoxy/gl.h>
#include "math_3d.h"

typedef struct
{
} ChessView3dPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ChessView3d, chess_view3d, GTK_TYPE_GL_AREA)

enum {
	PROP_0,
	N_PROPS
};

static GParamSpec *properties [N_PROPS];

ChessView3d *
chess_view3d_new (void)
{
	return g_object_new (CHESS_TYPE_VIEW3D, NULL);
}

static void
chess_view3d_finalize (GObject *object)
{
	ChessView3d *self = (ChessView3d *)object;
	ChessView3dPrivate *priv = chess_view3d_get_instance_private (self);

	G_OBJECT_CLASS (chess_view3d_parent_class)->finalize (object);
}

static void
chess_view3d_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
	ChessView3d *self = CHESS_VIEW3D (object);

	switch (prop_id)
	  {
	  default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  }
}

static void
chess_view3d_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
	ChessView3d *self = CHESS_VIEW3D (object);

	switch (prop_id)
	  {
	  default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  }
}

static void
chess_view3d_class_init (ChessView3dClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = chess_view3d_finalize;
	object_class->get_property = chess_view3d_get_property;
	object_class->set_property = chess_view3d_set_property;
}

static gboolean
render (GtkGLArea    *area,
        GdkGLContext *context)
{
	/* TODO: render things */
	return TRUE;
}

static void
chess_view3d_init (ChessView3d *self)
{
	g_signal_connect (self, "render", G_CALLBACK (render), NULL);
}
