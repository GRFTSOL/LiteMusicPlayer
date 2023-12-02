#include "MPlayerApp.h"
#include "MLCmd.h"


#undef DEFINE_CMD_ID
#define DEFINE_CMD_ID(uid) { #uid, uid, nullptr },

#undef DEFINE_CMD_ID_TIP
#define DEFINE_CMD_ID_TIP(uid, tooltip)   { #uid, uid, tooltip },

UIObjectIDDefinition g_uidDefinition[] = {
    MP_SKIN_IDS

    DEFINE_CMD_ID(ID_SKIN_START)
    DEFINE_CMD_ID(ID_ADD_RESULTS_TO_PLAYLIST_START)
    DEFINE_CMD_ID(ID_ADD_SELECTED_TO_PLAYLIST_START)


    { nullptr, 0, nullptr },
};
