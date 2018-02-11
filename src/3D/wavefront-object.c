#include "wavefront-object.h"
#include "../gl-util.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
  GString *location;
  gchar *name;
  bool genbuffers;
  GLuint vao;
  GLuint vbo;
  int num_tris;
  GLfloat (*verts)[3][8];
} WavefrontObjectPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WavefrontObject, wavefront_object, G_TYPE_INITIALLY_UNOWNED)

enum {
  PROP_0,
  PROP_LOCATION,
  PROP_NAME,
  PROP_VAO,
  PROP_VBO,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

void
load_wavefront_object (const char *filename,
                       char **name,
                       GLfloat (**verts)[3][8],
                       int *num_tris,
                       GError **error)
{
  FILE *file = NULL;
  GArray *temp_verts = g_array_new (false, false, sizeof (vec3_t));
  GArray *temp_uvs = g_array_new (false, false, sizeof (GLfloat[2]));
  GArray *temp_normals = g_array_new (false, false, sizeof (vec3_t));
  vec3_t center = vec3(0,0,0);

  GArray *tris = g_array_new (false, false, sizeof (int[3][3]));

  if ((file = fopen_gresource (filename, error)) == NULL) {
    goto end;
  }

  char *line = NULL;
  size_t length = 0;

  while (getline(&line, &length, file) != -1) {
    char *space = strchr (line, ' ');

    if (!space)
      continue;

    if (strncmp(line, "v", space - line) == 0) {
      vec3_t vertex;

      sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      g_array_append_val (temp_verts, vertex);

      center = v3_add (center, vertex);
    } else if (strncmp(line, "vt", space - line) == 0) {
      GLfloat uv[2];

      sscanf(line, "vt %f %f", &uv[0], &uv[1]);
      g_array_append_val (temp_uvs, uv);
    } else if (strncmp(line, "vn", space - line) == 0) {
      vec3_t normal;

      sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
      g_debug ("adding normal(%f,%f,%f)", normal.x, normal.y, normal.z);
      g_array_append_val (temp_normals, normal);
    } else if (strncmp(line, "f", space - line) == 0) {
      int face[3][3];

      if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
             &face[0][0], &face[0][1], &face[0][2],
             &face[1][0], &face[1][1], &face[1][2],
             &face[2][0], &face[2][1], &face[2][2]) != 9) {
        if (sscanf(line, "f %d//%d %d//%d %d//%d",
               &face[0][0], &face[0][2],
               &face[1][0], &face[1][2],
               &face[2][0], &face[2][2]) == 6) {
          face[0][1] = -1;
          face[1][1] = -1;
          face[2][1] = -1;
          g_array_append_val (tris, face);
        }
      } else {
        g_array_append_val (tris, face);
      }
    } else if (strncmp(line, "o", space - line) == 0) {
      sscanf(line, "o %ms", name);
    } else {
      line [strlen (line) - 1] = '\0';
      g_debug ("OBJ loader: skipping line '%s'", line);
    }
  }

  if (!feof(file)) {
    *error = g_error_new (g_quark_from_static_string ("OBJ loader"),
                          errno, "%s", strerror (errno));
    goto end;
  }

  center = v3_divs (center, temp_verts->len);

  /* Create the object.
   * 1. for all faces f in F:
   *  1a. for all points p in f: (vert_idx,uv_idx,normal_idx) = p
   *  1b. push new vec[3 + 2 + 3] into array of vertices
   * 2. convert
   */
  *verts = calloc (tris->len, sizeof ((*verts)[0]));
  *num_tris = tris->len;

  for (int i=0; i<tris->len; ++i) {
    int triangle[3][3];
    vec3_t v[3];
    GLfloat uv[3][2];
    vec3_t n[3];

    memcpy(triangle, ((int (*)[3][3])(tris->data))[i], sizeof triangle);

    for (int p=0; p<3; ++p) {
      v[p] = ((vec3_t *) temp_verts->data)[triangle[p][0] - 1];
      n[p] = ((vec3_t *) temp_normals->data)[triangle[p][2] - 1];

      if (triangle[p][1] >= 0) {
        GLfloat (*uvs)[2] = (void *) temp_uvs->data;

        uv[p][0] = uvs[triangle[p][1] - 1][0];
        uv[p][1] = uvs[triangle[p][1] - 1][1];
      }
    }

    vec3_t tri_center = v3_divs (v3_add (v3_add (v[0], v[1]), v[2]), 3);
    vec3_t normal = v3_sub (center, tri_center);

    /*
     * normal * ((v1 - v0) x (v2 - v0))
     */
    if (v3_dot (normal, v3_cross (v3_sub (v[1], v[0]), v3_sub (v[2], v[0]))) > 0) {
      const int p1 = 0;
      const int p2 = 2;
      vec3_t temp = v[p1];
      v[p1] = v[p2];
      v[p2] = temp;

      temp = n[p1];
      n[p1] = n[p2];
      n[p2] = temp;

      int temp2[3];
      memcpy(&temp2, &triangle[p1], sizeof temp2);
      memcpy(&triangle[p1], &triangle[p2], sizeof triangle[p1]);
      memcpy(&triangle[p2], &temp2, sizeof triangle[p2]);
      g_debug ("tri[%d]: swapping p%d and p%d", i, p1, p2);
    }

    for (int p=0; p<3; ++p) {
      v[p] = v3_sub (v[p], center);
      g_debug ("tri[%d]: adding point %d//%d", i, triangle[p][0], triangle[p][2]);
      g_debug ("tri[%d]: vertex(%f,%f,%f)", i, v[p].x, v[p].y, v[p].z);
      memcpy(&(*verts)[i][p][0], &v[p], sizeof(v[p]));
      if (triangle[p][1] >= 0) {
        g_debug ("tri[%d]: UV(%f,%f)", i, uv[p][0], uv[p][1]);
        memcpy(&(*verts)[i][p][3], &uv[p], sizeof(uv[p]));
      }
      g_debug ("tri[%d]: normal(%f,%f,%f)", i, n[p].x, n[p].y, n[p].z);
      memcpy(&(*verts)[i][p][5], &n[p], sizeof(n[p]));
    }
  }

end:
  g_array_unref (temp_verts);
  g_array_unref (temp_uvs);
  g_array_unref (temp_normals);

  g_array_unref (tris);
  if (file)
    fclose(file);
}

WavefrontObject *
wavefront_object_new (const char  *location,
                      GError     **error)
{
  WavefrontObject *obj;
  char *name = NULL;
  int num_tris = 0;
  GLfloat (*verts)[3][8] = NULL;

  load_wavefront_object (location, &name, &verts, &num_tris, error);
  if (!verts || num_tris == 0 || *error) {
    obj = NULL;
  } else {
    obj = g_object_new (WAVEFRONT_TYPE_OBJECT,
                        "location", location,
                        NULL);
    WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (obj);
    priv->name = name;
    priv->num_tris = num_tris;
    priv->verts = verts;
  }

  return obj;
}

static void
wavefront_object_finalize (GObject *object)
{
  WavefrontObject *self = (WavefrontObject *)object;
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  G_OBJECT_CLASS (wavefront_object_parent_class)->finalize (object);
}

static void
wavefront_object_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  WavefrontObject *self = WAVEFRONT_OBJECT (object);

  switch (prop_id)
    {
    case PROP_LOCATION:
      g_value_set_string (value, wavefront_object_get_location (self));
      break;
    case PROP_NAME:
      g_value_set_string (value, wavefront_object_get_name (self));
      break;
    case PROP_VAO:
      g_value_set_uint (value, wavefront_object_get_vao (self));
      break;
    case PROP_VBO:
      g_value_set_uint (value, wavefront_object_get_vbo (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wavefront_object_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  WavefrontObject *self = WAVEFRONT_OBJECT (object);
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_LOCATION:
      g_string_assign (priv->location, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wavefront_object_class_init (WavefrontObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = wavefront_object_finalize;
  object_class->get_property = wavefront_object_get_property;
  object_class->set_property = wavefront_object_set_property;

  properties [PROP_LOCATION] =
    g_param_spec_string ("location",
                         "Location",
                         "Location",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));

  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name",
                         NULL,
                         (G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS));

  properties [PROP_VAO] =
    g_param_spec_uint ("vao",
                       "Vao",
                       "Vertex array object",
                       0, ~((GLuint) 0), 0,
                       (G_PARAM_READABLE |
                        G_PARAM_STATIC_STRINGS));

  properties [PROP_VBO] =
    g_param_spec_uint ("vbo",
                       "Vbo",
                       "Vertex buffer object",
                       0, ~((GLuint) 0), 0,
                       (G_PARAM_READABLE |
                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
wavefront_object_init (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  priv->location = g_string_new ("");
}

const gchar *
wavefront_object_get_location (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  return priv->location->str;
}

const gchar *
wavefront_object_get_name (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  return priv->name;
}

void wavefront_object_generate_buffers (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  if (priv->genbuffers)
    return;

  glGenVertexArrays (1, &priv->vao);
  glBindVertexArray (priv->vao);

  glGenBuffers (1, &priv->vbo);
  glBindBuffer (GL_ARRAY_BUFFER, priv->vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof(priv->verts[0]) * priv->num_tris, &priv->verts[0], GL_STATIC_DRAW);

  priv->genbuffers = true;
}

void wavefront_object_hook_shader_attributes (WavefrontObject *self,
                                              GLuint           program)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);
  GLint attribPtr;

  if (!priv->genbuffers)
    wavefront_object_generate_buffers (self);

  glBindVertexArray (priv->vao);

  attribPtr = glGetAttribLocation (program, "position");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 3 /* 3 floats = x,y,z */,
                         GL_FLOAT, GL_FALSE, sizeof (priv->verts[0][0]),
                         0 /* offset of the vec3 */);

  attribPtr = glGetAttribLocation (program, "texcoord");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 2 /* 2 floats = u,v */,
                         GL_FLOAT, GL_FALSE, sizeof (priv->verts[0][0]),
                         (void *)(3 * sizeof (priv->verts[0][0][0])) /* offset of the UV elements */);

  attribPtr = glGetAttribLocation (program, "normal");
  glEnableVertexAttribArray (attribPtr);
  glVertexAttribPointer (attribPtr, 3 /* 3 floats = x,y,z */,
                         GL_FLOAT, GL_FALSE, sizeof (priv->verts[0][0]),
                         (void *)(5 * sizeof (priv->verts[0][0][0])) /* offset of the normal vector elements */);
}

void wavefront_object_render (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  glDrawArrays (GL_TRIANGLES, 0, (sizeof (priv->verts[0]) * priv->num_tris)  / sizeof (priv->verts[0][0]));
}

GLuint wavefront_object_get_vao (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  return priv->vao;
}

GLuint wavefront_object_get_vbo (WavefrontObject *self)
{
  WavefrontObjectPrivate *priv = wavefront_object_get_instance_private (self);

  return priv->vbo;
}
