extern "C"
{
#include "FLAC/all.h"
//#include "plugin_common/all.h"
};

/*
 *  constants
 */

class CMDFlac;

int FLAC_plugin__decoder_init(CMDFlac *pMDFlac);
void FLAC_plugin__seek(CMDFlac *pMDFlac);

extern void FLAC_plugin__show_error(const char *message,...);
