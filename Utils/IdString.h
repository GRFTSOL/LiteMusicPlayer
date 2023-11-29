#pragma once

#ifndef _OUR_LIB_ID_STRING_INC_
#define _OUR_LIB_ID_STRING_INC_


struct IdToString {
    uint32_t                    dwId;
    cstr_t                      szId;
};

#define DEFINE_ID(dwId)     {dwId, #dwId}
#define DEFINE_IDT(dwId)    {dwId, #dwId}

uint32_t stringToID(IdToString Ids[], cstr_t szID, uint32_t dwDefault);
cstr_t idToString(IdToString Ids[], uint32_t nID, cstr_t szDefaultID);

// szValue: szIDx | szIDy | szIDz
uint32_t getCombinationValue(IdToString Ids[], cstr_t szValue);

// Convert value to string: XXX | YYY | ZZZ
void getCombinationStrValue(IdToString Ids[], uint32_t value, string &strValue);

#endif // _OUR_LIB_ID_STRING_INC_
