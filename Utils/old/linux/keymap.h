#pragma once

#ifndef _LINUX_GTK2_KEYMAP_INC_
#define _LINUX_GTK2_KEYMAP_INC_

#include <gdk/gdkkeysyms.h>


#define VK_LBUTTON          0x01
#define VK_RBUTTON          0x02
#define VK_CANCEL           0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define VK_BACK             GDK_BackSpace
#define VK_TAB              GDK_Tab

#define VK_CLEAR            GDK_Clear
#define VK_RETURN           GDK_Return

#define VK_SHIFT            GDK_Shift_L
#define VK_CONTROL          GDK_Control_L
#define VK_MENU             GDK_Menu
#define VK_PAUSE            GDK_Pause
#define VK_CAPITAL          GDK_Caps_Lock
//
// #define VK_KANA           0x15
// #define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
// #define VK_HANGUL         0x15
// #define VK_JUNJA          0x17
// #define VK_FINAL          0x18
// #define VK_HANJA          0x19
// #define VK_KANJI          0x19

#define VK_ESCAPE           GDK_Escape

// #define VK_CONVERT        0x1C
// #define VK_NONCONVERT     0x1D
// #define VK_ACCEPT         0x1E
// #define VK_MODECHANGE     0x1F

#define VK_SPACE            GDK_space
#define VK_PRIOR            GDK_Prior
#define VK_NEXT             GDK_Next
#define VK_END              GDK_End
#define VK_HOME             GDK_Home
#define VK_LEFT             GDK_Left
#define VK_UP               GDK_Up
#define VK_RIGHT            GDK_Right
#define VK_DOWN             GDK_Down
#define VK_SELECT           GDK_Select
#define VK_PRINT            GDK_Print
#define VK_EXECUTE          GDK_Execute
// #define VK_SNAPSHOT       0x2C
#define VK_INSERT           GDK_Insert
#define VK_DELETE           GDK_Delete
#define VK_HELP             GDK_Help

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

// #define VK_LWIN           0x5B
// #define VK_RWIN           0x5C
// #define VK_APPS           0x5D

// #define VK_NUMPAD0        0x60
// #define VK_NUMPAD1        0x61
// #define VK_NUMPAD2        0x62
// #define VK_NUMPAD3        0x63
// #define VK_NUMPAD4        0x64
// #define VK_NUMPAD5        0x65
// #define VK_NUMPAD6        0x66
// #define VK_NUMPAD7        0x67
// #define VK_NUMPAD8        0x68
// #define VK_NUMPAD9        0x69
// #define VK_MULTIPLY       0x6A
#define VK_ADD              0x6B
#define VK_SEPARATOR        0x6C
#define VK_SUBTRACT         0x6D
#define VK_DECIMAL          0x6E
#define VK_DIVIDE           0x6F
#define VK_F1               GDK_F1
#define VK_F2               GDK_F2
#define VK_F3               GDK_F3
#define VK_F4               GDK_F4
#define VK_F5               GDK_F5
#define VK_F6               GDK_F6
#define VK_F7               GDK_F7
#define VK_F8               GDK_F8
#define VK_F9               GDK_F9
#define VK_F10              GDK_F10
#define VK_F11              GDK_F11
#define VK_F12              GDK_F12
#define VK_F13              GDK_F13
#define VK_F14              GDK_F14
#define VK_F15              GDK_F15
#define VK_F17              GDK_F17
#define VK_F18              GDK_F18
#define VK_F19              GDK_F19
#define VK_F20              GDK_F20
#define VK_F21              GDK_F21
#define VK_F22              GDK_F22
#define VK_F23              GDK_F23
#define VK_F24              GDK_F24

#define VK_NUMLOCK          GDK_Num_Lock
#define VK_SCROLL           GDK_Scroll_Lock

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT           GDK_Shift_L
#define VK_RSHIFT           GDK_Shift_R
#define VK_LCONTROL         GDK_Control_L
#define VK_RCONTROL         GDK_Control_R
#define VK_LMENU            GDK_Menu
#define VK_RMENU            GDK_Menu

#endif // _LINUX_GTK2_KEYMAP_INC_
