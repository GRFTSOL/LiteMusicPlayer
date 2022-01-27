#ifndef __SENDFILETO_H__
#define __SENDFILETO_H__

#include <mapi.h>

class CMailFileTo
{
public:
    bool sendMail(HWND hWndParent, cstr_t szSendToName, cstr_t szSendToAddress, cstr_t strAttachmentFileName, cstr_t strSubject)
    {
        if (isEmptyString(strAttachmentFileName))
            return false;

//         if (!hWndParent || !::isWindow(hWndParent))
//             return false;

        HINSTANCE hMAPI = ::LoadLibrary("MAPI32.DLL");
        if (!hMAPI)
            return false;

        ULONG (PASCAL *sendMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
        (FARPROC&)sendMail = GetProcAddress(hMAPI, "MAPISendMail");
        if (!sendMail)
        {
            FreeLibrary(hMAPI);
            return false;
        }

        char szFileName[_MAX_PATH] = { 0 };
        char szPath[_MAX_PATH] = { 0 };
        char szSubject[_MAX_PATH] = { 0 };

        convertStr2(strAttachmentFileName, -1, szFileName, CountOf(szFileName));
        convertStr2(strAttachmentFileName, -1, szPath, CountOf(szPath));
        convertStr2(strSubject, -1, szSubject, CountOf(szSubject));
        //strcpy_safe(szFileName, CountOf(szFileName), strAttachmentFileName);
        //strcpy_safe(szPath, CountOf(szPath), strAttachmentFileName);
        //strcpy_safe(szSubject, CountOf(szSubject), strSubject);

        MapiRecipDesc    sendTo;
        char            szSendToNameAnsi[MAX_PATH] = { 0 };
        char            szSendToAddressAnsi[MAX_PATH] = { 0 };
        convertStr2(szSendToName, -1, szSendToNameAnsi, CountOf(szSendToNameAnsi));
        convertStr2(szSendToAddress, -1, szSendToAddressAnsi, CountOf(szSendToAddressAnsi));

        memset(&sendTo, 0, sizeof(sendTo));
        sendTo.ulRecipClass = MAPI_TO;
        sendTo.lpszName = szSendToNameAnsi;
        sendTo.lpszAddress = szSendToAddressAnsi;

        MapiFileDesc fileDesc;
        ::ZeroMemory(&fileDesc, sizeof(fileDesc));
        fileDesc.nPosition = (ULONG)-1;
        fileDesc.lpszPathName = szPath;
        fileDesc.lpszFileName = szFileName;

        MapiMessage message;
        ::ZeroMemory(&message, sizeof(message));
        message.lpszSubject = szSubject;
        message.nFileCount = 1;
        message.lpFiles = &fileDesc;

        message.lpRecips = &sendTo;
        message.nRecipCount = 1;

        int nError = sendMail(0, (ULONG_PTR)hWndParent, &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

        FreeLibrary(hMAPI);

        if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
            return false;

        return true;
    }
};

#endif
