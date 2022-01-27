
public class SkinWndEvent {

	// Return true, the command will not be processed down.
	public boolean onCommand(int nCmdId) { return false; }
	public void onCreate(SkinWnd skinWnd) { }
	public void onDestroy() { }
	public void onSize(int cx, int cy) { }
	public void onMove(int x, int y) { }
	public void onActivate(boolean bActived) { }
	public void onMouseActive(boolean bMouseActive) { }

}

public class UIObject {

}

public class SkinWnd {

	public native UIObject getUIObject(int nId);
	public native void setUIObjVisible(int nId, boolean visible);
	public native boolean getUIObjVisible(int nId);

	public native void setUIObjProperty(int nId, String name, String value);

	public native void setProperty(String name, String value);

	public native void exeCommand(int nCmdId);

	public native int getWndPositionX();
	public native int getWndPositionY();
	public native int getWndWidth();
	public native int getWndHeight();

	public native void setWndPosition(int x, int y);
	public native void moveWindow(int x, int y, int width, int height);

	public native int registerTimer(int timeOut);
	public native void unregisterTimer(int nTimerId);

	public native void startAnimation(int nAnimationId);
	public native void stopAnimation(int nAnimationId);

}

public class SkinSystemEvent {

	public void onInit() { }
	public void onDestory() { }
	public void onSkinWndCreate(SkinWnd skinWnd) { }
	public void onSkinWndDestory(SkinWnd skinWnd) { }

}

public class SkinSystem {

	public static native boolean registerSkinSystemEvent(SkinSystemEvent event);
	public static native void unRegisterSkinSystemEvent(SkinSystemEvent event);

	public static native boolean registerSkinWndEvent(String strWndName, SkinWndNofityEvent event);
	public static native void unRegisterSkinWndEvent(SkinWndNofityEvent event);

	public static native SkinWnd getSkinWnd(String skinWndName);

	public static native int getCommandID(String string);

	public static void writeProfileInt(String szValueName, int nValue);
	public static void writeProfileString(String szValueName, String szValue);

	public static int getProfileInt(String szValueName, int nDefault);
	public static String getProfileString(String szValueName, String szDefault);

}
