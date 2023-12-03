#pragma once

#include "../Utils/UtilsTypes.h"


class CColor;
class CRawGraph;
class CRawImage;


#ifndef _WIN32
// the dwFlags of drawTextEx
#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_SINGLELINE       0x00000020
#define DT_NOCLIP           0x00000100
#define DT_END_ELLIPSIS     0x00008000
#endif

#define DT_PREFIX_TEXT      0x02000000

