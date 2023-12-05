#pragma once

#include "../Utils/Utils.h"
#include "../GfxRaw/GfxRawTypes.h"

#ifdef _WIN32
#define MK_COMMAND  MK_CONTROL

enum {
  VK_A                      = 'A',
  VK_S                      = 'S',
  VK_D                      = 'D',
  VK_F                      = 'F',
  VK_H                      = 'H',
  VK_G                      = 'G',
  VK_Z                      = 'Z',
  VK_X                      = 'X',
  VK_C                      = 'C',
  VK_V                      = 'V',
  VK_B                      = 'B',
  VK_Q                      = 'Q',
  VK_W                      = 'W',
  VK_E                      = 'E',
  VK_R                      = 'R',
  VK_Y                      = 'Y',
  VK_T                      = 'T',
  VK_1                      = '1',
  VK_2                      = '2',
  VK_3                      = '3',
  VK_4                      = '4',
  VK_6                      = '6',
  VK_5                      = '5',
  VK_EQUAL                  = VK_ADD,
  VK_9                      = '9',
  VK_7                      = '7',
  VK_MINUS                  = VK_SUBTRACT,
  VK_8                      = '8',
  VK_0                      = '0',
  VK_RIGHT_BRACKET          = VK_OEM_6,
  VK_O                      = 'O',
  VK_U                      = 'U',
  VK_LEFT_BRACKET           = VK_OEM_4,
  VK_I                      = 'I',
  VK_P                      = 'P',
  VK_L                      = 'L',
  VK_J                      = 'J',
  VK_QUOTE                  = 0xDE, // VK_OEM7,
  VK_K                      = 'K',
  VK_SEMI_COLON             = VK_OEM_1,
  VK_BACK_SLASH             = VK_OEM_5,
  VK_COMMA                  = VK_OEM_COMMA,
  VK_SLASH                  = VK_OEM_2,
  VK_N                      = 'N',
  VK_M                      = 'M',
  VK_PERIOD                 = VK_OEM_PERIOD,
  VK_GRAVE                  = VK_OEM_3,
//   VK_KEYPAD_DECIMAL         = 0x41,
//   VK_KEYPAD_MULTIPLY        = 0x43,
//   VK_KEYPAD_PLUS            = 0x45,
//   VK_KEYPAD_CLEAR           = 0x47,
//   VK_KEYPAD_DIVIDE          = 0x4B,
//   VK_KEYPAD_ENTER           = 0x4C,
//   VK_KEYPAD_MINUS           = 0x4E,
//   VK_KEYPAD_EQUAL           = 0x51,
  VK_KEYPAD_0               = VK_NUMPAD0,
  VK_KEYPAD_1               = VK_NUMPAD1,
  VK_KEYPAD_2               = VK_NUMPAD2,
  VK_KEYPAD_3               = VK_NUMPAD3,
  VK_KEYPAD_4               = VK_NUMPAD4,
  VK_KEYPAD_5               = VK_NUMPAD5,
  VK_KEYPAD_6               = VK_NUMPAD6,
  VK_KEYPAD_7               = VK_NUMPAD7,
  VK_KEYPAD_8               = VK_NUMPAD8,
  VK_KEYPAD_9               = VK_NUMPAD9
};

#endif


#ifndef _WIN32
struct POINT {
    int                         x;
    int                         y;
};
#endif

struct CPoint : POINT {
    CPoint() { x = 0; y = 0; }
    CPoint(int x, int y) { this->x = x; this->y = y; }

    bool operator == (const CPoint &other) const {
        return x == other.x && y == other.y;
    }

};

using VecPoints = std::vector<CPoint>;

#ifndef _WIN32
struct SIZE {
    int                         cx;
    int                         cy;
};
#endif

struct CSize : SIZE {

    CSize() { cx = 0; cy = 0; }
    CSize(int cx, int cy) { this->cx = cx; this->cy = cy; }

    bool operator == (const CSize &other) const {
        return cx == other.cx && cy == other.cy;
    }

};

#ifndef _WIN32
struct RECT {
    int                         left;
    int                         top;
    int                         right;
    int                         bottom;
};
#endif

struct CRect : RECT {
    CRect() { left = 0, top = 0, right = 0, bottom = 0; }
    CRect(int left, int top, int right, int bottom) {
        this->left = left; this->top = top; this->right = right; this->bottom = bottom;
    }

    CRect(const RECT& other) {
        left = other.left; top = other.top; right = other.right; bottom = other.bottom;
    }

    void setLTRB(int left, int top, int right, int bottom) {
        this->left = left; this->top = top;
        this->right = right; this->bottom = bottom;
    }

    void setLTWH(int left, int top, int width, int height) {
        this->left = left; this->top = top;
        this->right = left + width; this->bottom = top + height;
    }

    bool empty() const {
        return left >= right || top >= bottom;
    }

    void setEmpty() {
        left = top = right = bottom = 0;
    }

    void offsetRect(int xOffset, int yOffset) {
        left += xOffset;
        top += yOffset;
        right += xOffset;
        bottom += yOffset;
    }

    void deflate(int cx, int cy) {
        left += cx; right -= cx;
        top += cy; bottom -= cy;
    }

    void inflate(int cx, int cy) {
        left -= cx; right += cx;
        top -= cy; bottom += cy;
    }

    bool equal(const CRect &other) const {
        return left == other.left && right == other.right && top == other.top && bottom == other.bottom;
    }

    bool intersect(const CRect &a, const CRect &b) {
        if (a.empty() || b.empty() ||
            (a.left >= b.right) || (b.left >= a.right) ||
            (a.top >= b.bottom) || (b.top >= a.bottom)) {
            setEmpty();
            return false;
        }

        left = std::max(a.left, b.left);
        right = std::min(a.right, b.right);
        top = std::max(a.top, b.top);
        bottom = std::min(a.bottom, b.bottom);
        return true;
    }

    void intersect(const CRect &other) {
        if (other.left > left) left = other.left;
        if (other.top > top) top = other.top;
        if (other.right < right) right = other.right;
        if (other.bottom < bottom) bottom = other.bottom;

        if (empty()) setEmpty();
    }

    bool ptInRect(const CPoint &pt) const {
        return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
    }

    int width() const { return right - left; }
    int height() const { return bottom - top; }

    bool operator == (const RECT &other) const {
        return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
    }

    bool operator != (const RECT &other) const {
        return left != other.left || top != other.top || right != other.right || bottom != other.bottom;
    }

};

inline CRect makeRectLTWH(int left, int top, int width, int height)
    { return CRect(left, top, left + width, top + height); }
