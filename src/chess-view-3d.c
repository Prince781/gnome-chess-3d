#include "chess-view-3d.h"
#include <epoxy/gl.h>
#include "math_3d.h"
#include "gl-util.h"

typedef struct
{
  GLuint glsl_program;
  GLuint vshader, fshader;

  mat4_t model, view, proj;

  GHashTable *models;
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

static void
realize (GtkWidget *self)
{
  ChessView3dPrivate *priv;
  int win_width = gtk_widget_get_allocated_width (self);
  int win_height = gtk_widget_get_allocated_height (self);
  GError *error = NULL;

  priv = chess_view3d_get_instance_private (CHESS_VIEW3D(self));
  g_assert (priv != NULL);

  gtk_gl_area_make_current (GTK_GL_AREA (self));

  gtk_gl_area_set_has_depth_buffer (GTK_GL_AREA(self), TRUE);
  gtk_gl_area_set_has_stencil_buffer (GTK_GL_AREA(self), TRUE);

  priv->model = m4_rotation(M_PI, vec3(0.0f,0.0f,1.0f));

  /* view matrix:
   * arg1 = position of camera
   * arg2 = center of view
   * arg3 = direction up. Note that because UP is along the z-axis, this causes
   * (x,y)-plane to be perpendicular to "up"
   */
  priv->view = m4_look_at(vec3(1.2f,1.2f,1.2f), vec3(0.0f,0.0f,0.0f), vec3(0.0f,0.0f,1.0f));

  /* projection matrix:
   * arg1 = field of view in degrees
   * arg2 = aspect ratio
   * arg3 = near distance
   * arg4 = far distance
   */
  priv->proj = m4_perspective(60.0f, (float)win_width / win_height, 1.0f, 10.0f);

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

  /* load pawn */
  struct Obj3D *pawn_obj;

  pawn_obj = load_OBJ ("/org/gnome/chess/3d/pieces/pawn.obj", &error);

  if (error) {
    g_error ("failed to load pawn: %s", error->message);
    return;
  }

  /* create VAO */
  glGenVertexArrays(1, &pawn_obj->vao);
  glBindVertexArray(pawn_obj->vao);

  /* generate VBO */
  glGenBuffers(1, &pawn_obj->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, pawn_obj->vbo);
  glBufferData(GL_ARRAY_BUFFER, pawn_obj->verts_size * 8, pawn_obj->verts, GL_STATIC_DRAW);

  /* now we set up some inputs to the vertex shader */
  GLint attribPtr;

  attribPtr = glGetAttribLocation (priv->glsl_program, "position");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 3 /* 3 floats = x,y,z */,
                         GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat),
                         0 /* offset of the vec3 */);

  attribPtr = glGetAttribLocation (priv->glsl_program, "texcoord");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 2 /* 2 floats = u,v */,
                         GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat),
                         (void *)(3 * sizeof (GLfloat)) /* offset of the UV elements */);

  attribPtr = glGetAttribLocation (priv->glsl_program, "normal");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 3 /* 3 floats = x,y,z */,
                         GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat),
                         (void *)(5 * sizeof (GLfloat)) /* offset of the normal vector elements */);

  g_hash_table_insert (priv->models, pawn_obj->name, pawn_obj);
}

static gboolean
render (GtkGLArea    *area,
        GdkGLContext *context)
{
	/* TODO: render things */
	return TRUE;
}

static void
unrealize (GtkWidget *self)
{
  /* TODO: uninitialize */
}

static void
chess_view3d_init (ChessView3d *self)
{
  ChessView3dPrivate *priv;

  priv = chess_view3d_get_instance_private (self);

  priv->models = g_hash_table_new (g_str_hash, g_str_equal);

  g_signal_connect (self, "realize", G_CALLBACK (realize), NULL);
	g_signal_connect (self, "render", G_CALLBACK (render), NULL);
  g_signal_connect (self, "unrealize", G_CALLBACK (unrealize), NULL);
}
