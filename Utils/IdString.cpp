#include "UtilsTypes.h"
#include "IdString.h"


uint32_t stringToID(IdToString Ids[], cstr_t szID, uint32_t dwDefault) {
    for (IdToString *ptr = Ids; ptr->szId != nullptr; ptr++) {
        if (strcasecmp(szID, ptr->szId) == 0) {
            return ptr->dwId;
        }
    }

    return dwDefault;
}

cstr_t idToString(IdToString Ids[], uint32_t nID, cstr_t szDefaultID) {
    for (IdToString *ptr = Ids; ptr->szId != nullptr; ptr++) {
        if (nID == ptr->dwId) {
            return ptr->szId;
        }
    }

    return szDefaultID;
}

uint32_t getCombinationValue(IdToString Ids[], cstr_t szValue) {
    vector<StringView> vStr;

    StringView str(szValue);
    str.split('|', vStr);

    uint32_t dwFlags = 0;
    for (auto &str : vStr) {
        dwFlags |= stringToID(Ids, str.trim(' ').toString().c_str(), 0);
    }

    return dwFlags;
}

void getCombinationStrValue(IdToString Ids[], uint32_t value, string &strValue) {
    for (int i = 0; Ids[i].szId != nullptr; i++) {
        if ((Ids[i].dwId & value) == Ids[i].dwId) {
            if (!strValue.empty()) {
                strValue += "|";
            }
            strValue += Ids[i].szId;
        }
    }
}
