#pragma once

#ifndef _LYRICSOUTPLUGIN_H_
#define _LYRICSOUTPLUGIN_H_

interface IMLyrOutHost {
    virtual int getLineCount() = 0;
    virtual bool getLyricsOfLine(int nLine, char szLyrics[], int nBuffLen, int &beginTime, int &endTime) = 0;
    virtual int getCurLine() = 0;
    virtual int getPlayPos() = 0;
    virtual bool getMediaFile(char szFile[], int nBuffLen) = 0;
#ifdef _WIN32
    virtual HWND getMainWnd() = 0;
#endif
};

#define ML_LYR_OUT_VERSION  2

interface ILyricsOut {
    enum {
        NOTIF_CUR_LINE              = 1, // onCurLineChanged
        NOTIF_HALF_CUR_LINE         = 1 << 1, // OnNextLine
        NOTIF_PLAY_POS              = 1 << 2, // onPlayPos
    };

#ifdef _WIN32
    virtual void config(HWND hWndParent) = 0;
    virtual void about(HWND hWndParent) = 0;
#endif
    virtual cstr_t getDescription() = 0;

    // uNotifyFlag, return NOTIF_CUR_LINE, NOTIF_...
    virtual bool onInit(unsigned int *uNotifyFlag) = 0;

    virtual void onCurLineChanged(int nLine, cstr_t szLyrics, int beginTime, int endTime) = 0;
    virtual void onHalfOfCurLine(int nLine) = 0;
    virtual void onPlayPos(int uPos) = 0;
    virtual void onLyricsChanged() = 0;
    virtual void onSongChanged(cstr_t szArtist, cstr_t szTitle, int nDuration) = 0;
    virtual void onQuit() = 0;

};



#endif // _LYRICSOUTPLUGIN_H_
