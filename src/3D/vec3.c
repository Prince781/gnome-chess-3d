#include "vec3.h"

G_DEFINE_BOXED_TYPE (Vec3, vec3, vec3_copy, vec3_free)

Vec3 *
vec3_new (vec3_t vector)
{
  Vec3 *self;

  self = g_slice_new0 (Vec3);
  self->vec = vector;

  return self;
}

Vec3 *
vec3_copy (Vec3 *self)
{
  Vec3 *copy;

  g_return_val_if_fail (self, NULL);

  copy = vec3_new (self->vec);

  return copy;
}

void
vec3_free (Vec3 *self)
{
  g_return_if_fail (self);

  g_slice_free (Vec3, self);
}
