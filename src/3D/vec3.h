#ifndef VEC3_H
#define VEC3_H

#include <glib-object.h>
#include "../math_3d.h"

G_BEGIN_DECLS

#define VEC3_TYPE (vec3_get_type())

typedef struct _Vec3 Vec3;

struct _Vec3
{
  vec3_t vec;
};

Vec3     *vec3_new   (vec3_t vector);
Vec3     *vec3_copy  (Vec3 *self);
void      vec3_free  (Vec3 *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (Vec3, vec3_free)

G_END_DECLS

#endif /* VEC3_H */

