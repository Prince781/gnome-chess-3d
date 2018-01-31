/* -*- Mode: vala; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2010-2014 Robert Ancell
 * Copyright (C) 2015-2016 Sahil Sareen
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

public class ChessView3D : Gtk.GLArea
{
    private int border = 6;
    private int square_size;
    private int selected_square_size;
    private string loaded_theme_name = "";

    private ChessScene _scene;
    public ChessScene scene
    {
        get { return _scene; }
        set
        {
            _scene = value;
            // _scene.changed.connect (scene_changed_cb);
            queue_draw ();
        }
    }

    private double border_size
    {
        get { return square_size / 2; }
    }

    public ChessView3D ()
    {
        add_events (Gdk.EventMask.BUTTON_PRESS_MASK | Gdk.EventMask.BUTTON_RELEASE_MASK);
    }

    public override bool configure_event (Gdk.EventConfigure event)
    {
        int short_edge = int.min (get_allocated_width (), get_allocated_height ());

        square_size = (int) Math.floor ((short_edge - 2 * border) / 9.0);
        var extra = square_size * 0.1;
        if (extra < 3)
            extra = 3;
        selected_square_size = square_size + 2 * (int) (extra + 0.5);

        return true;
    }
}