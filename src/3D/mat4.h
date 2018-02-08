#ifndef MAT4_H
#define MAT4_H

#include <glib-object.h>
#include "../math_3d.h"

G_BEGIN_DECLS

GType mat4_get_type (void);

#define MAT4_TYPE (mat4_get_type())

typedef struct _Mat4 Mat4;

struct _Mat4
{
  mat4_t mat;
};

Mat4     *mat4_new   (mat4_t matrix);
Mat4     *mat4_copy  (Mat4 *self);
void      mat4_free  (Mat4 *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (Mat4, mat4_free)

G_END_DECLS

#endif /* MAT4_H */

