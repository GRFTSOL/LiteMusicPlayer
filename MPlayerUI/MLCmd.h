#pragma once

//
// Command ID of UIObject in SkinWnd:
//
enum ML_CUSTOM_COMMAND {
    CMD_TOPMOST                 = CMD_BASE_END,
    CMD_CLICK_THROUGH,
    CMD_RATE_LYR,

    CMD_PLAYPAUSE,
    CMD_PLAY,
    CMD_PAUSE,
    CMD_STOP,
    CMD_BACKWARD,
    CMD_FORWARD,
    CMD_J_PREV_LINE,                 // Jump to previous line of lyrics
    CMD_J_NEXT_LINE,                 // Jump to next line of lyrics

    // Lyrics display options
    CMD_FONT_SIZE_INC,
    CMD_FONT_SIZE_DEC,
    CMD_CLR_PREV_HUE,
    CMD_CLR_NEXT_HUE,
    CMD_LYR_HIGH_CLR_LIST,
    CMD_MENU_STATIC_LYR,

    ID_TB_LYR_SYNC,
    ID_TB_LYR_TXT,
    ID_TB_LYR_EDIT,

    // lyrics editor command
    CMD_TOGGLE_LYR_EDIT_TOOLBAR,
    CMD_JUMP,
    CMD_INSERTTAG,
    CMD_DEL_TAG,
    CMD_INSERTTAG_DOWN,
    CMD_FORWARD_REMAIN_LINES,        // Forward the remaining lyrics 0.2s
    CMD_BACKWARD_REMAIN_LINES,       // Backward the remaining lyrics 0.2s
    CMD_FORWARD_CUR_LINE,
    CMD_BACKWARD_CUR_LINE,
    CMD_AUTO_FILL_LYR_INFO,
    CMD_NEW_LRC,
    CMD_OPEN_LRC,
    CMD_SAVE_LRC,
    CMD_SAVE_LRC_AS,
    CMD_UPLOAD_LYR,
    CMD_EDIT_HELP,
    CMD_LYR_EDITOR,
    CMD_EXTERNAL_LYR_EDIT,
    CMD_SAVE_LYR_IN_SONG_FILE,

    // lyrics text edit command
    CMD_EDIT_UNDO,
    CMD_EDIT_REDO,
    CMD_EDIT_CUT,
    CMD_EDIT_COPY,
    CMD_EDIT_PASTE,
    CMD_EDIT_DELETE,
    CMD_EDIT_FIND,
    CMD_EDIT_FINDNEXT,
    CMD_EDIT_REPLACE,
    CMD_EDIT_LYR_TAG,

    CMD_REMOVE_ALL_TAG,
    CMD_REMOVE_BLANK_LINE,
    CMD_TRIM_WHITESPACE,
    CMD_REMOVE_UNSYNC_LINES,
    CMD_CAPITALIZE_LEADING_LETTER,
    CMD_LYRICS_TO_LOWERCASE,

    CMD_FORWARD_LYRICS,
    CMD_BACKWARD_LYRICS,
    CMD_DISPLAY_OPT,
    CMD_HELP,
    CMD_PREVIOUS,
    CMD_NEXT,

    // Player control command/ID
    CMD_SEEK,
    CMD_VOL_INC,
    CMD_VOL_DEC,
    CMD_VOLUME,
    CMD_MUTE,
    ID_PLAYLIST,
    CMD_SHUFFLE,
    CMD_LOOP,
    CMD_LOOP_OFF,
    CMD_LOOP_ALL,
    CMD_LOOP_TRACK,
    CMD_EQ,
    CMD_RATE,

    CMD_TXT_TITLE,
    CMD_TXT_POS,
    CMD_TXT_VOL,
    CMD_TXT_STATE,
    CMD_TXT_INFO,

    ID_STEREO_STAT,

    // playlist command
    CMD_PL_UP,
    CMD_PL_DOWN,
    CMD_PL_ADD_FILE,
    CMD_PL_ADD_DIR,
    CMD_PL_ADD_URL,
    CMD_PL_OPEN_FILE,
    CMD_PL_OPEN_DIR,
    CMD_PL_OPEN_URL,
    CMD_PL_DEL,
    CMD_PL_PROPERTY,
    CMD_PL_NEW,
    CMD_PL_LOAD,
    CMD_PL_SAVE,
    CMD_PL_LIST,

    // pocket pc cmd
    CMD_TOGGLE_SCREEEN,              // turn screen on/off

    // Media Guide View
    CMD_MG_TREE_GUIDE,
    CMD_MG_MEDIA_LIST,

    // media library
    CMD_ML_GUIDE,
    CMD_ML_BACK,
    ID_ML_MEDIA_LIST,

    ID_AD_TXT_LINK,
    ID_AD_IMG_LINK,

    ID_ALBUMART,                     // album art control

    // other cmd
    CMD_TOGGLE_MP,
    CMD_SHOW_MAIN_WND,
    CMD_PREFERENCES,

    CMD_RELOAD_LYR,

    CMD_FLOATING_LYRICS,

    CMD_RATE_LYR_1,
    CMD_RATE_LYR_2,
    CMD_RATE_LYR_3,
    CMD_RATE_LYR_4,
    CMD_RATE_LYR_5,
    CMD_LDO_KARAOKE,
    CMD_ABOUT,
    CMD_ADJUST_HUE,
    CMD_BR_ALBUM_ART,
    CMD_LOGIN_VIA_IE,
    CMD_APPLY_ACCOUNT,
    CMD_EMAIL,
    CMD_WEBHOME,
    CMD_ANTIAlIAS,
    CMD_LDO_NORMAL,
    CMD_LDO_FADE_IN,
    CMD_LDO_FADEOUT_BG,
    CMD_LDO_AUTO,
    CMD_LDS_MULTI_LINE,
    CMD_LDS_STATIC_TXT,
    CMD_LDS_TWO_LINE,
    CMD_LDS_SINGLE_LINE,
    CMD_LDS_VOBSUB,

    // Commands for long error text
    CMD_NO_SUITTABLE_LYRICS,
    CMD_INSTRUMENTAL_MUSIC,
    CMD_SEARCH_LYR_SUGGESTIONS,

    // Commands below haven't any ID associated.
    CMD_SHOW_ERR_RESULT,
};

extern UIObjectIDDefinition g_uidDefinition[];
