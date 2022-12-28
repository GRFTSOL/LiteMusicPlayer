#include "MPMsg.h"


void cmdLineAnalyse(cstr_t szCmdLine, vector<string> &vCmdLine) {
    cstr_t szBeg, szEnd;
    char chSign;
    string str;

    szBeg = szCmdLine;

    while (*szBeg) {
        if (*szBeg == '"') {
            chSign = '"';
            szBeg++;
        } else {
            chSign = ' ';
        }

        szEnd = szBeg;
        while (*szEnd != '\0' && *szEnd != chSign) {
            szEnd++;
        }

        str = "";
        str.append(szBeg, szEnd);
        if (!str.empty()) {
            vCmdLine.push_back(str);
        }

        if (*szEnd == '"') {
            szEnd++;
        }
        while (*szEnd == ' ') {
            szEnd++;
        }

        szBeg = szEnd;
    }
}

cstr_t cmdLineNext(cstr_t szCmdLine) {
    char chSign;

    if (*szCmdLine == '"') {
        chSign = '"';
        szCmdLine++;
    } else {
        chSign = ' ';
    }

    while (*szCmdLine != '\0' && *szCmdLine != chSign) {
        szCmdLine++;
    }
    if (*szCmdLine == '"') {
        szCmdLine++;
    }
    while (*szCmdLine == ' ') {
        szCmdLine++;
    }

    return szCmdLine;
}

void sendCommandLine(HWND hWnd, cstr_t szCmdLine) {
    COPYDATASTRUCT copyData;

    copyData.dwData = ML_SEND_CMD_LINE;
    copyData.lpData = (void *)szCmdLine;
    copyData.cbData = sizeof(char) * (strlen(szCmdLine) + 1);

    sendMessage(hWnd, WM_COPYDATA, ML_SEND_CMD_LINE, (LPARAM)&copyData);
}

void sendActivateMainWnd(HWND hWnd) {
    COPYDATASTRUCT copyData;

    copyData.dwData = ML_ACTIVATE;
    copyData.lpData = "";
    copyData.cbData = 1;

    sendMessage(hWnd, WM_COPYDATA, ML_ACTIVATE, (LPARAM)&copyData);
}
