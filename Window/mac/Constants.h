#pragma once

#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define SW_HIDE             0
#define SW_MAXIMIZE         3
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_RESTORE          9
#define SW_SHOWMINIMIZED    10
#define SW_SHOWMINNOACTIVE  SW_SHOWMINIMIZED
#define SW_SHOWMAXIMIZED    11
#define SW_SHOWNOACTIVATE   SW_SHOW

// Cocoa Events.h 中转换而来
enum {
  VK_A                      = 0x00,
  VK_S                      = 0x01,
  VK_D                      = 0x02,
  VK_F                      = 0x03,
  VK_H                      = 0x04,
  VK_G                      = 0x05,
  VK_Z                      = 0x06,
  VK_X                      = 0x07,
  VK_C                      = 0x08,
  VK_V                      = 0x09,
  VK_B                      = 0x0B,
  VK_Q                      = 0x0C,
  VK_W                      = 0x0D,
  VK_E                      = 0x0E,
  VK_R                      = 0x0F,
  VK_Y                      = 0x10,
  VK_T                      = 0x11,
  VK_1                      = 0x12,
  VK_2                      = 0x13,
  VK_3                      = 0x14,
  VK_4                      = 0x15,
  VK_6                      = 0x16,
  VK_5                      = 0x17,
  VK_EQUAL                  = 0x18,
  VK_9                      = 0x19,
  VK_7                      = 0x1A,
  VK_MINUS                  = 0x1B,
  VK_8                      = 0x1C,
  VK_0                      = 0x1D,
  VK_RIGHT_BRACKET          = 0x1E,
  VK_O                      = 0x1F,
  VK_U                      = 0x20,
  VK_LEFT_BRACKET           = 0x21,
  VK_I                      = 0x22,
  VK_P                      = 0x23,
  VK_L                      = 0x25,
  VK_J                      = 0x26,
  VK_QUOTE                  = 0x27,
  VK_K                      = 0x28,
  VK_SEMI_COLON             = 0x29,
  VK_BACK_SLASH             = 0x2A,
  VK_COMMA                  = 0x2B,
  VK_SLASH                  = 0x2C,
  VK_N                      = 0x2D,
  VK_M                      = 0x2E,
  VK_PERIOD                 = 0x2F,
  VK_GRAVE                  = 0x32,
  VK_KEYPAD_DECIMAL         = 0x41,
  VK_KEYPAD_MULTIPLY        = 0x43,
  VK_KEYPAD_PLUS            = 0x45,
  VK_KEYPAD_CLEAR           = 0x47,
  VK_KEYPAD_DIVIDE          = 0x4B,
  VK_KEYPAD_ENTER           = 0x4C,
  VK_KEYPAD_MINUS           = 0x4E,
  VK_KEYPAD_EQUAL           = 0x51,
  VK_KEYPAD_0               = 0x52,
  VK_KEYPAD_1               = 0x53,
  VK_KEYPAD_2               = 0x54,
  VK_KEYPAD_3               = 0x55,
  VK_KEYPAD_4               = 0x56,
  VK_KEYPAD_5               = 0x57,
  VK_KEYPAD_6               = 0x58,
  VK_KEYPAD_7               = 0x59,
  VK_KEYPAD_8               = 0x5B,
  VK_KEYPAD_9               = 0x5C
};

/* keycodes for keys that are independent of keyboard layout*/
enum {
  VK_RETURN                 = 0x24,
  VK_TAB                    = 0x30,
  VK_SPACE                  = 0x31,
  VK_DELETE                 = 0x33,
  VK_ESCAPE                 = 0x35,
  VK_COMMAND                = 0x37,
  VK_SHIFT                  = 0x38,
  VK_CAPS_LOCK              = 0x39,
  VK_OPTION                 = 0x3A,
  VK_CONTROL                = 0x3B,
  VK_RIGHT_COMMAND          = 0x36,
  VK_RIGHT_SHIFT            = 0x3C,
  VK_RIGHT_OPTION           = 0x3D,
  VK_RIGHT_CONTROL          = 0x3E,
  VK_FUNCTION               = 0x3F,
  VK_F17                    = 0x40,
  VK_VOLUME_UP              = 0x48,
  VK_VOLUME_DOWN            = 0x49,
  VK_MUTE                   = 0x4A,
  VK_F18                    = 0x4F,
  VK_F19                    = 0x50,
  VK_F20                    = 0x5A,
  VK_F5                     = 0x60,
  VK_F6                     = 0x61,
  VK_F7                     = 0x62,
  VK_F3                     = 0x63,
  VK_F8                     = 0x64,
  VK_F9                     = 0x65,
  VK_F11                    = 0x67,
  VK_F13                    = 0x69,
  VK_F16                    = 0x6A,
  VK_F14                    = 0x6B,
  VK_F10                    = 0x6D,
  VK_F12                    = 0x6F,
  VK_F15                    = 0x71,
  VK_HELP                   = 0x72,
  VK_HOME                   = 0x73,
  VK_PAGE_UP                = 0x74,
  VK_FORWARD_DELETE         = 0x75,
  VK_F4                     = 0x76,
  VK_END                    = 0x77,
  VK_F2                     = 0x78,
  VK_PAGE_DOWN              = 0x79,
  VK_F1                     = 0x7A,
  VK_LEFT                   = 0x7B,
  VK_RIGHT                  = 0x7C,
  VK_DOWN                   = 0x7D,
  VK_UP                     = 0x7E
};

#define VK_ALT              VK_OPTION
#define VK_MENU             VK_ALT

// #define VK_INSERT           114

#define VK_PRIOR            VK_PAGE_UP
#define VK_NEXT             VK_PAGE_DOWN
#define VK_BACK             VK_DELETE

#define VK_ADD              VK_EQUAL
#define VK_SUBTRACT         VK_MINUS

// Modifier Flags
#define MK_SHIFT            (1 << 17) // NSShiftKeyMask
#define MK_CONTROL          (1 << 18) // NSControlKeyMask
#define MK_ALT              (1 << 19) // NSAlternateKeyMask
#define MK_COMMAND          (1 << 20) // NSCommandKeyMask

#define WHEEL_DELTA         50

#define MB_OK               0x00000000L
#define MB_OKCANCEL         0x00000001L
#define MB_ABORTRETRYIGNORE 0x00000002L
#define MB_YESNOCANCEL      0x00000003L
#define MB_YESNO            0x00000004L
#define MB_RETRYCANCEL      0x00000005L

#define MB_ICONHAND         0x00000010L
#define MB_ICONQUESTION     0x00000020L
#define MB_ICONEXCLAMATION  0x00000030L
#define MB_ICONASTERISK     0x00000040L
#define MB_ICONWARNING      MB_ICONEXCLAMATION
#define MB_ICONERROR        MB_ICONHAND
#define MB_ICONINFORMATION  MB_ICONASTERISK
#define MB_ICONSTOP         MB_ICONHAND

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define IDCLOSE             8
#define IDHELP              9

#define MK_LBUTTON          1
