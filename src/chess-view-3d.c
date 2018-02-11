#include "chess-view-3d.h"
#include <epoxy/gl.h>
#include "math_3d.h"
#include "gl-util.h"
#include "3D/chess3d-camera.h"
#include "3D/wavefront-object.h"

#define MAX_LIGHTS  100

typedef enum {
  LIGHT_DIRECTIONAL = 0,
  LIGHT_POINT       = 1
} LightType;

typedef struct {
  float intensity;
  vec3_t color;
  float radius;
  LightType type;
  vec3_t position;
} Light;

typedef struct
{
  GLuint glsl_program;
  GLuint vshader, fshader;

  mat4_t model;

  /* HashTable<Wavefront.Object> */
  GHashTable *models;

  /* HashTable<Chess3d.GameObject> */
  GHashTable *game_objects;

  Chess3dCamera *camera;

  int num_lights;
  Light lights[MAX_LIGHTS];
} ChessView3dPrivate;

/* TODO: multiple lights */

G_DEFINE_TYPE_WITH_PRIVATE (ChessView3d, chess_view3d, GTK_TYPE_GL_AREA)

enum {
	PROP_0,
  PROP_MODELS,
  PROP_CAMERA,
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
  ChessView3dPrivate *priv = chess_view3d_get_instance_private (self);

	switch (prop_id)
	  {
    case PROP_MODELS:
      g_hash_table_ref (priv->models);
      g_value_set_boxed (value, priv->models);
      break;
    case PROP_CAMERA:
      g_value_set_object (value, priv->camera);
      break;
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

  properties [PROP_MODELS] =
    g_param_spec_boxed ("models",
                        "Models",
                        "Models",
                        G_TYPE_HASH_TABLE,
                        (G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS));

  properties [PROP_CAMERA] =
    g_param_spec_object ("camera",
                         "Camera",
                         "Camera",
                         CHESS3D_TYPE_CAMERA,
                         (G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static gboolean
queue_render (GtkWidget     *self,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  gtk_gl_area_queue_render (GTK_GL_AREA (self));
  return G_SOURCE_CONTINUE;
}

static void
realize (GtkWidget *self)
{
  ChessView3dPrivate *priv;
  GError *error = NULL;

  priv = chess_view3d_get_instance_private (CHESS_VIEW3D(self));
  g_assert (priv != NULL);

  gtk_gl_area_make_current (GTK_GL_AREA (self));

  gtk_gl_area_set_has_depth_buffer (GTK_GL_AREA(self), TRUE);
  gtk_gl_area_set_has_stencil_buffer (GTK_GL_AREA(self), TRUE);

  priv->model = m4_rotation(0, vec3(0.0f,1.0f,0.0f));

  /* load shaders */
  priv->vshader = shader_new ("/org/gnome/chess/3d/shaders/shader.glslv",
                              GL_VERTEX_SHADER, &error);
  if (error) {
    g_error ("failed to load shader: %s", error->message);
    return;
  }
  shader_compile (priv->vshader, &error);
  if (error) {
    g_error ("failed to compile shader: %s", error->message);
    return;
  }

  priv->fshader = shader_new ("/org/gnome/chess/3d/shaders/shader.glslf",
                              GL_FRAGMENT_SHADER, &error);
  if (error) {
    g_error ("failed to load shader: %s", error->message);
    return;
  }
  shader_compile (priv->fshader, &error);
  if (error) {
    g_error ("failed to compile shader: %s", error->message);
    return;
  }

  /* create program, attach shaders, and link */
  priv->glsl_program = glCreateProgram();

  glAttachShader (priv->glsl_program, priv->vshader);
  glAttachShader (priv->glsl_program, priv->fshader);

  glBindFragDataLocation (priv->glsl_program, 0, "outColor");

  shader_program_link (priv->glsl_program, &error);
  if (error) {
    g_error ("failed to link program: %s", error->message);
    return;
  }

  glUseProgram(priv->glsl_program);

  shader_program_set_vec3 (priv->glsl_program, "lightColor", vec3 (1.f, 1.f, 1.f));
  shader_program_set_vec3 (priv->glsl_program, "lightDirection", vec3 (0.f, 1.f, 0.f));

  /* load pawn */
  WavefrontObject *pawn_obj;
  const gchar *name;

  pawn_obj = wavefront_object_new ("/org/gnome/chess/3d/pieces/pawn.obj", &error);

  if (error) {
    g_error ("failed to load pawn: %s", error->message);
    return;
  }

  wavefront_object_generate_buffers (pawn_obj);
  wavefront_object_hook_shader_attributes (pawn_obj, priv->glsl_program);

  name = wavefront_object_get_name (pawn_obj);
  g_hash_table_insert (priv->models, g_strdup (name), g_object_ref_sink (pawn_obj));

  Chess3dModel *model;
  Chess3dGameObject *pawn;

  model = chess3d_model_new (pawn_obj);
  pawn = chess3d_game_object_new ("Pawn");

  chess3d_game_object_set_model (pawn, model);
  name = chess3d_game_object_get_name (pawn);

  g_hash_table_insert (priv->game_objects, g_strdup (name), g_object_ref_sink (pawn));

  gtk_widget_add_tick_callback (self, queue_render, NULL, NULL);
}

static gboolean
render (GtkGLArea    *area,
        GdkGLContext *context)
{
  ChessView3dPrivate *priv;
  GHashTableIter iter;
  gpointer key;
  gpointer val;
  int win_width, win_height;
  mat4_t view, proj;

  priv = chess_view3d_get_instance_private (CHESS_VIEW3D (area));

  win_width = gtk_widget_get_allocated_width (GTK_WIDGET (area));
  win_height = gtk_widget_get_allocated_height (GTK_WIDGET (area));

  view = chess3d_camera_get_view (priv->camera);
  proj = chess3d_camera_get_projection (priv->camera, (float) win_width / win_height);

  shader_program_set_mat4 (priv->glsl_program, "view", view);
  shader_program_set_mat4 (priv->glsl_program, "proj", proj);

  glEnable (GL_DEPTH_TEST);

  glClearColor (0.2f, 0.2f, 0.2f, 1.f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  g_hash_table_iter_init (&iter, priv->game_objects);
  while (g_hash_table_iter_next (&iter, &key, &val)) {
    Chess3dGameObject *game_obj = CHESS3D_GAME_OBJECT (val);
    Chess3dModel *model;

    if ((model = chess3d_game_object_get_model (game_obj))) {
      priv->model = m4_mul (priv->model, m4_rotation_y (M_PI / 180));

      shader_program_set_mat4 (priv->glsl_program, "model", priv->model);
      shader_program_set_vec3 (priv->glsl_program, "overrideColor", vec3 (1.f,1.f,1.f));

      wavefront_object_render (chess3d_model_get_object (model));
    }
  }

  glDisable (GL_DEPTH_TEST);
  glFlush ();
	return TRUE;
}

static void
unrealize (GtkWidget *self)
{
  /* TODO: uninitialize */
}

static gboolean
mousedown_cb (GtkWidget *self,
              GdkEvent  *event)
{
  GdkEventButton *evb;
  GdkEventMotion *evm;

  evb = &event->button;
  evm = &event->motion;

  g_debug ("mousedown event (%lf, %lf)!", evm->x, evm->y);

  return TRUE;
}

static gboolean
mousemove_cb (GtkWidget *self,
              GdkEvent  *event)
{
  GdkEventButton *evb;
  GdkEventMotion *evm;

  evb = &event->button;
  evm = &event->motion;

  g_debug ("mousemove event (%lf, %lf)!", evm->x, evm->y);

  return TRUE;
}

static gboolean
mouseup_cb (GtkWidget *self,
            GdkEvent  *event)
{
  GdkEventButton *evb;
  GdkEventMotion *evm;

  evb = &event->button;
  evm = &event->motion;

  g_debug ("mouseup event (%lf, %lf)!", evm->x, evm->y);

  return TRUE;
}

static void
chess_view3d_init (ChessView3d *self)
{
  ChessView3dPrivate *priv;

  priv = chess_view3d_get_instance_private (self);

  priv->models = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  priv->game_objects = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  priv->camera = chess3d_camera_new (60.f, 1.f, 100.f);
  chess3d_game_object_set_position ((Chess3dGameObject *) priv->camera, vec3 (0, 0, -4));

  gtk_widget_set_events (GTK_WIDGET (self),
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_MOTION_MASK
                         | GDK_BUTTON_RELEASE_MASK);

  g_signal_connect (self, "realize", G_CALLBACK (realize), NULL);
	g_signal_connect (self, "render", G_CALLBACK (render), NULL);
  g_signal_connect (self, "unrealize", G_CALLBACK (unrealize), NULL);

  g_signal_connect (self, "button-press-event", G_CALLBACK (mousedown_cb), NULL);
  g_signal_connect (self, "motion-notify-event", G_CALLBACK (mousemove_cb), NULL);
  g_signal_connect (self, "button-release-event", G_CALLBACK (mouseup_cb), NULL);
}
