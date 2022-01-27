#pragma once

//
// Command line handler.
//

#define ML_SEND_CMD_LINE        0xF30B4730
#define ML_ACTIVATE                0x803C27CA

#ifdef _MPLAYER
#define MSG_WND_CLASS_NAME        "ZikiPlayerMsgWnd"
#else
#define MSG_WND_CLASS_NAME        "MiniLyricsMsgWnd"
#endif

void cmdLineAnalyse(cstr_t szCmdLine, vector<string> &vCmdLine);

cstr_t cmdLineNext(cstr_t szCmdLine);

void sendCommandLine(HWND hWnd, cstr_t szCmdLine);

void sendActivateMainWnd(HWND hWnd);

