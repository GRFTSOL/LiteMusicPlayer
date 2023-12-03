#ifndef GfxRaw_win32_UnicodeRange_h
#define GfxRaw_win32_UnicodeRange_h

#pragma once

// The following constants define unicode subranges
// values below kRangeNum must be continuous so that we can map to
// lang group directly.
// all ranges we care about should be defined under 32, that allows
// us to store range using bits of a PRUint32

// frequently used range definitions
const uint8_t   kRangeCyrillic =    ED_CYRILLIC_WINDOWS;//0;
const uint8_t   kRangeGreek    =    ED_GREEK_WINDOWS;//1;
const uint8_t   kRangeTurkish  =    ED_TURKISH_WINDOWS;//2;
const uint8_t   kRangeHebrew   =    ED_HEBREW_WINDOWS;//3;
const uint8_t   kRangeArabic   =    ED_ARABIC;//4;
const uint8_t   kRangeBaltic   =    ED_BALTIC_WINDOWS;//5;
const uint8_t   kRangeThai     =    ED_THAI;//6;
const uint8_t   kRangeKorean   =    ED_KOREAN;//7;
const uint8_t   kRangeJapanese =    ED_JAPANESE_SHIFT_JIS;//8;
const uint8_t   kRangeSChinese =    ED_GB2312;//9;
const uint8_t   kRangeTChinese =    ED_BIG5;//10;

const uint8_t   kRangeDevanagari =  20;    // 11
const uint8_t   kRangeTamil    =   21;    // 12

//const uint8_t   kRangeSpecificItemNum = 13;

//range/rangeSet grow to this place 13-29

const uint8_t   kRangeSetStart  =  30;    // range set definition starts from here
const uint8_t   kRangeSetLatin  =  ED_LATIN9_ISO;
const uint8_t   kRangeSetCJK    =  ED_GB2312;
const uint8_t   kRangeSetEnd    =  31;   // range set definition ends here

// less frequently used range definition
const uint8_t   kRangeSurrogate            = 32;
const uint8_t   kRangePrivate              = 33;
const uint8_t   kRangeMisc                 = 34;
const uint8_t   kRangeUnassigned           = 35;
const uint8_t   kRangeSyriac               = 36;
const uint8_t   kRangeThaana               = 37;
const uint8_t   kRangeBengali              = 38;
const uint8_t   kRangeGurmukhi             = 39;
const uint8_t   kRangeGujarati             = 40;
const uint8_t   kRangeOriya                = 41;
const uint8_t   kRangeTelugu               = 42;
const uint8_t   kRangeKannada              = 43;
const uint8_t   kRangeMalayalam            = 44;
const uint8_t   kRangeSinhala              = 45;
const uint8_t   kRangeLao                  = 46;
const uint8_t   kRangeTibetan              = 47;
const uint8_t   kRangeMyanmar              = 48;
const uint8_t   kRangeGeorgian             = 49;
const uint8_t   kRangeEthiopic             = 50;
const uint8_t   kRangeCherokee             = 51;
const uint8_t   kRangeAboriginal           = 52;
const uint8_t   kRangeOghamRunic           = 53;
const uint8_t   kRangeKhmer                = 54;
const uint8_t   kRangeMongolian            = 55;
const uint8_t   kRangeMathOperators        = 56;
const uint8_t   kRangeMiscTechnical        = 57;
const uint8_t   kRangeControlOpticalEnclose = 58;
const uint8_t   kRangeBoxBlockGeometrics   = 59;
const uint8_t   kRangeMiscSymbols          = 60;
const uint8_t   kRangeDingbats             = 61;
const uint8_t   kRangeBraillePattern       = 62;
const uint8_t   kRangeYi                   = 63;
const uint8_t   kRangeCombiningDiacriticalMarks = 64;
const uint8_t   kRangeArmenian             = 65;

const uint8_t   kRangeTableBase             = 128;    //values over 127 are reserved for internal use only
const uint8_t   kRangeTertiaryTable         = 145; // leave room for 16 subtable 
                                            // indices (kRangeTableBase + 1 ..
                                            // kRangeTableBase + 16)



extern uint32_t findCharUnicodeRange(WCHAR ch);

extern CharEncodingType FindCharEncUnicodeRange(WCHAR ch);


#endif // !defined(GfxRaw_win32_UnicodeRange_h)
