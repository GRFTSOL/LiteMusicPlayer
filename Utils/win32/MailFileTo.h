#pragma once

#ifndef __SENDFILETO_H__
#define __SENDFILETO_H__

#include <mapi.h>


inline bool sendMail(HWND hWndParent, cstr_t szSendToName, cstr_t szSendToAddress, cstr_t strAttachmentFileName, cstr_t strSubject) {
    if (isEmptyString(strAttachmentFileName)) {
        return false;
    }

    HINSTANCE hMAPI = ::LoadLibraryA("MAPI32.DLL");
    if (!hMAPI) {
        return false;
    }

    ULONG (PASCAL *SendMail)(ULONG, ULONG_PTR, MapiMessageW*, FLAGS, ULONG);
    (FARPROC&)SendMail = GetProcAddress(hMAPI, "MAPISendMailW");
    if (!SendMail) {
        FreeLibrary(hMAPI);
        return false;
    }

    MapiRecipDescW sendTo;
    memset(&sendTo, 0, sizeof(sendTo));
    sendTo.ulRecipClass = MAPI_TO;
    utf16string utf16SendToName = utf8ToUCS2(szSendToName);
    utf16string utf16SendToAddress = utf8ToUCS2(szSendToAddress);
    sendTo.lpszName = (LPWSTR)utf16SendToName.c_str();
    sendTo.lpszAddress = (LPWSTR)utf16SendToAddress.c_str();

    MapiFileDescW fileDesc;
    ::ZeroMemory(&fileDesc, sizeof(fileDesc));
    fileDesc.nPosition = (ULONG)-1;
    utf16string utf16FileName = utf8ToUCS2(strAttachmentFileName);
    fileDesc.lpszPathName = (LPWSTR)utf16FileName.c_str();
    fileDesc.lpszFileName = (LPWSTR)utf16FileName.c_str();

    MapiMessageW message;
    ::ZeroMemory(&message, sizeof(message));
    utf16string utf16Subject = utf8ToUCS2(strSubject);
    message.lpszSubject = (LPWSTR)utf16Subject.c_str();
    message.nFileCount = 1;
    message.lpFiles = &fileDesc;

    message.lpRecips = &sendTo;
    message.nRecipCount = 1;

    int nError = SendMail(0, (ULONG_PTR)hWndParent, &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

    FreeLibrary(hMAPI);

    if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE) {
        return false;
    }

    return true;
}

#endif
