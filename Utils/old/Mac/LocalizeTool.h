// LocalizeTool.h: interface for the CLocalizeTool class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

cstr_t _TL(cstr_t szString);
#define _TLT _TL
#define _TL _TL

#define _TLS(szString)
#define _TLM(szString)        szString

//#define ERROR2STR_LOCAL(nError)    (g_LangTool.toLocalString(Error2Str(nError)))
string removePrefixOfAcckey(cstr_t szStr);
