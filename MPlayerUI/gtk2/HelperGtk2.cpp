// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "../MPlayerApp.h"
#include "../Helper.h"

void execute(Window *pWnd, cstr_t szExe, cstr_t szParam)
{
}

bool setClipboardText(Window *pWnd, cstr_t szText)
{
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), szText, strlen(szText));
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), szText, strlen(szText));
}

// 向用户显示消息框
int messageOut(cstr_t lpText, cstr_t szSaveDefault,Window *pWnd/* = nullptr*/, uint32_t uType/* = MB_ICONINFORMATION*/, cstr_t lpCaption/* = nullptr*/)
{
    GtkWidget        *msgDlg;
    int                nRet;

    GtkMessageType        msgType;
    GtkButtonsType        btType;
    if (isFlagSet(uType, MB_OK))
        btType = GTK_BUTTONS_OK;
    else if (isFlagSet(uType, MB_YESNO))
        btType = GTK_BUTTONS_YES_NO;
    else if (isFlagSet(uType, MB_OKCANCEL))
        btType = GTK_BUTTONS_OK_CANCEL;
    else
        btType = GTK_BUTTONS_OK;

    if (isFlagSet(uType, MB_ICONWARNING))
        msgType = GTK_MESSAGE_WARNING;
    else if (isFlagSet(uType, MB_ICONQUESTION))
        msgType = GTK_MESSAGE_QUESTION;
    else if (isFlagSet(uType, MB_ICONINFORMATION))
        msgType = GTK_MESSAGE_INFO;
    else if (isFlagSet(uType, MB_ICONERROR))
        msgType = GTK_MESSAGE_ERROR;
    else
        msgType = GTK_MESSAGE_INFO;

    msgDlg = gtk_message_dialog_new(pWnd ? GTK_WINDOW(pWnd->getHandle()) : nullptr, GTK_DIALOG_MODAL, msgType, btType,  "%s", lpText);
    if (lpCaption)
        gtk_window_set_title(GTK_WINDOW(msgDlg), lpCaption);
    nRet = gtk_dialog_run(GTK_DIALOG (msgDlg));
    gtk_widget_destroy (msgDlg);

    return nRet;
}

bool SHDeleteFile(cstr_t szFile, Window *pWndParent)
{
    return deleteFile(szFile);
}

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent)
{
    // return copyFile(szSrcFile, szTargFile, true);
    return false;
}

bool setFileNoReadOnly(cstr_t szFile)
{
    return false;
}

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort)
{
    return false;
}

void getNotepadEditor(string &strEditor)
{
}

uint32_t getSecCount()
{
    time_t        curTime;
    return time(&curTime);
}
