#ifndef CHESS_VIEW_3D_H
#define CHESS_VIEW_3D_H

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHESS_TYPE_VIEW3D (chess_view3d_get_type())

G_DECLARE_DERIVABLE_TYPE (ChessView3d, chess_view3d, CHESS, VIEW3D, GtkGLArea)

struct _ChessView3dClass
{
	GtkGLAreaClass parent;
};

ChessView3d *chess_view3d_new (void);

G_END_DECLS

#endif