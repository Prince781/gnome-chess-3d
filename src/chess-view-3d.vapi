[CCode (cheader_filename = "chess-view-3d.h")]
public class ChessView3d : Gtk.GLArea {
	[CCode (construct_function = "chess_view3d_new")]
	public ChessView3d ();
}