
public class MiniLyricsSkinWndEvent extends SkinWndEvent {

	public SkinWnd mSkinWnd;

	// Return true, the command will not be passed down.
	public boolean onCommand(int nCmdId)
	{
		// UnitTest.message("onCommand()");
		// UnitTest.message(nCmdId);
		
		if (nCmdId == MiniLyrics.ID_HIDE_WND_FRAME)
		{
			if (MiniLyrics.nHideFrameOnLostFocus > 0)
			{
				MiniLyrics.nHideFrameOnLostFocus = 0;
				mSkinWnd.setProperty("TranslucencyAlpha", "228");
			}
			else
			{
				MiniLyrics.nHideFrameOnLostFocus = 1;
				mSkinWnd.setProperty("TranslucencyAlpha", "0");
			}

			SkinSystem.writeProfileInt("HideFrameOnLostFocus", MiniLyrics.nHideFrameOnLostFocus);
		
			// mSkinWnd.setProperty("TranslucencyAlphaOnActive", "228");
			// mSkinWnd.setProperty("TranslucencyAlphaOnHover", "228");
		}
		else if (nCmdId == MiniLyrics.ID_MOUSE_HOVER_SHOW_WND)
		{
			if (MiniLyrics.nShowFrameOnMouseHover > 0)
			{
				MiniLyrics.nShowFrameOnMouseHover = 0;
				mSkinWnd.setProperty("TranslucencyAlphaOnHover", "0");
			}
			else
			{
				MiniLyrics.nShowFrameOnMouseHover = 1;
				mSkinWnd.setProperty("TranslucencyAlphaOnHover", "228");
			}

			SkinSystem.writeProfileInt("ShowFrameOnMouseHover", MiniLyrics.nShowFrameOnMouseHover);
		}

		return false;
	}
	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
		UnitTest.message("onCreate()");

		if (MiniLyrics.nHideFrameOnLostFocus > 0)
			mSkinWnd.setProperty("TranslucencyAlpha", "0");
		else
			mSkinWnd.setProperty("TranslucencyAlpha", "228");

		if (MiniLyrics.nShowFrameOnMouseHover > 0)
			mSkinWnd.setProperty("TranslucencyAlphaOnHover", "228");
		else
			mSkinWnd.setProperty("TranslucencyAlphaOnHover", "0");
	}

/*	public void onDestroy()
	{
		UnitTest.message("onDestroy()");
	}*/

	public void onActivate(boolean bActived)
	{
		if (bActived)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			// UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			// UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

	public void onMouseActive(boolean bMouseActive)
	{
		if (bMouseActive)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			// UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			// UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

}

public class MiniLyrics
{
	public static int ID_PLAYPAUSE;
	public static int CMD_AUTO_SHOW_PL;
	public static int CMD_AUTO_SHOW_LYR;
	public static int CMD_SHOW_INFO_TEXT;
	public static int CID_TOGGLE_LYR_PL;
	public static int CID_BT_TOGGLE_PL_LYR;
	public static int CID_AM_HIDE_PLAY;
	public static int CID_AM_SHOW_PLAY;
	public static int ID_HIDE_WND_FRAME;
	public static int ID_MOUSE_HOVER_SHOW_WND;
	public static int nHideFrameOnLostFocus, nShowFrameOnMouseHover;

	public static void main()
	{
		// UnitTest.message("MiniLyrics main()");

		ID_PLAYPAUSE = SkinSystem.getCommandID("ID_PLAYPAUSE");
		CMD_AUTO_SHOW_PL = SkinSystem.getCommandID("CMD_AUTO_SHOW_PL");
		CMD_AUTO_SHOW_LYR = SkinSystem.getCommandID("CMD_AUTO_SHOW_LYR");
		CMD_SHOW_INFO_TEXT = SkinSystem.getCommandID("CMD_SHOW_INFO_TEXT");
		CID_TOGGLE_LYR_PL = SkinSystem.getCommandID("CID_TOGGLE_LYR_PL");
		CID_BT_TOGGLE_PL_LYR = SkinSystem.getCommandID("CID_BT_TOGGLE_PL_LYR");
		CID_AM_HIDE_PLAY = SkinSystem.getCommandID("CID_AM_HIDE_PLAY");
		CID_AM_SHOW_PLAY = SkinSystem.getCommandID("CID_AM_SHOW_PLAY");
		ID_HIDE_WND_FRAME = SkinSystem.getCommandID("ID_HIDE_WND_FRAME");
		ID_MOUSE_HOVER_SHOW_WND = SkinSystem.getCommandID("ID_MOUSE_HOVER_SHOW_WND");

		nHideFrameOnLostFocus = SkinSystem.getProfileInt("HideFrameOnLostFocus", 1);
		nShowFrameOnMouseHover = SkinSystem.getProfileInt("ShowFrameOnMouseHover", 1);

		static MiniLyricsSkinWndEvent	skinWndEvent = new MiniLyricsSkinWndEvent();
		
		SkinSystem.registerSkinWndEvent("MainWnd", skinWndEvent);
	}
}
