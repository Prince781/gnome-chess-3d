#include "mat4.h"

G_DEFINE_BOXED_TYPE (Mat4, mat4, mat4_copy, mat4_free)

Mat4 *
mat4_new (mat4_t matrix)
{
  Mat4 *self;

  self = g_slice_new0 (Mat4);
  self->mat = matrix;

  return self;
}

Mat4 *
mat4_copy (Mat4 *self)
{
  Mat4 *copy;

  g_return_val_if_fail (self, NULL);

  copy = mat4_new (self->mat);

  return copy;
}

void
mat4_free (Mat4 *self)
{
  g_return_if_fail (self);

  g_slice_free (Mat4, self);
}
