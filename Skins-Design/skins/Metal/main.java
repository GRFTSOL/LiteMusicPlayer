
public class MainWndEvent extends SkinWndEvent {

	public SkinWnd mSkinWnd;

	// Return true, the command will not be passed down.
	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
		// UnitTest.message("onCreate()");
	}
	
	public boolean onCommand(int nCmdId)
	{
		if (nCmdId == CID_HIDE_PLAYLIST) {
			mSkinWnd->setUIObjVisible(CID_LYRICS_AREA, false);
		}

		return false;
	}

	public void onActivate(boolean bActived)
	{
		if (bActived)
		{
			mSkinWnd.stopAnimation(DHPlayer.CID_AM_HIDE_LYR_TB);
			mSkinWnd.startAnimation(DHPlayer.CID_AM_SHOW_LYR_TB);
			// UnitTest.message("CID_AM_SHOW_LYR_TB");
		}
		else
		{
			mSkinWnd.stopAnimation(DHPlayer.CID_AM_SHOW_LYR_TB);
			mSkinWnd.startAnimation(DHPlayer.CID_AM_HIDE_LYR_TB);
			// UnitTest.message("CID_AM_HIDE_LYR_TB");
		}
	}

	public void onMouseActive(boolean bMouseActive)
	{
		if (bMouseActive)
		{
			mSkinWnd.stopAnimation(DHPlayer.CID_AM_HIDE_LYR_TB);
			mSkinWnd.startAnimation(DHPlayer.CID_AM_SHOW_LYR_TB);
			// UnitTest.message("CID_AM_SHOW_LYR_TB");
		}
		else
		{
			mSkinWnd.stopAnimation(DHPlayer.CID_AM_SHOW_LYR_TB);
			mSkinWnd.startAnimation(DHPlayer.CID_AM_HIDE_LYR_TB);
			// UnitTest.message("CID_AM_HIDE_LYR_TB");
		}
	}

}

public class DHPlayer
{
	public static int CID_AM_HIDE_LYR_TB;
	public static int CID_AM_SHOW_LYR_TB;

	public static int CID_HIDE_PLAYLIST;
	public static int CID_LYRICS_AREA;

	public static void main()
	{
		// UnitTest.message("DHPlayer main()");

		CID_AM_HIDE_LYR_TB = SkinSystem.getCommandID("CID_AM_HIDE_LYR_TB");
		CID_AM_SHOW_LYR_TB = SkinSystem.getCommandID("CID_AM_SHOW_LYR_TB");
		CID_HIDE_PLAYLIST = SkinSystem.getCommandID("CID_HIDE_PLAYLIST");
		CID_LYRICS_AREA = SkinSystem.getCommandID("CID_LYRICS_AREA");

		static MainWndEvent	skinWndEvent = new MainWndEvent();

		SkinSystem.registerSkinWndEvent("MainWnd", skinWndEvent);
	}
}
