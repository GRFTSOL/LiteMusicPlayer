#include "Player.h"
#include "../Skin/Skin.h"
#include "MPSkinTimeCtrl.h"


string formatTime(int nTimeSec) {
    char szTime[64];
    int nHour, nMinute, nSec;

    nSec = nTimeSec % 60;
    nMinute = nTimeSec / 60;
    nHour = nMinute / 60;
    nMinute = nMinute % 60;
    if (nHour > 0) {
        snprintf(szTime, CountOf(szTime), "%02d:%02d:%02d", nHour, nMinute, nSec);
    } else {
        snprintf(szTime, CountOf(szTime), "%02d:%02d", nMinute, nSec);
    }

    return szTime;
}

//////////////////////////////////////////////////////////////////////

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinPicText>, "PlayingTime")

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinStaticText>, "PlayingTimeText")
