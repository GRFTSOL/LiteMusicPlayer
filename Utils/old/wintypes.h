#pragma once

#ifndef _WIN_TYPE_H
#define _WIN_TYPE_H

#ifdef _LINUX_GTK2
#include "linux/keymap.h"
#endif


#if !defined(WIN32)

#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

#define LPSTR_TEXTCALLBACK     ((char *) -1L)

#define _stricmp            strcasecmp
#define strncasecmp         strncasecmp

#define _stscanf            sscanf

#define fopen               fopen
#define _fgetts             fgets

#define _mbschr             strchr

/* String functions */

#define strchr              strchr
#define _tcscspn            strcspn
#define strncat             strncat
#define strncpy             strncpy
#define _tcspbrk            strpbrk
#define strrchr             strrchr
#define _tcsspn             strspn
#define strstr              strstr
#define _tcstok             strtok

#define _tcsnset            _strnset
#define _tcsrev             _strrev
#define _tcsset             _strset

#define strcmp              strcmp
#define strcasecmp          _stricmp
#define strncmp             strncmp
#define strncmp             strncmp
#define strncasecmp         strncasecmp
#define strcasecmp          strcasecmp
#define strncasecmp         strncasecmp

#define atoi                atoi
#define itoa                itoa
#define atof                atof

#define _tcscat             strcat
#define _tcscpy             strcpy
#define _tcsdup             _strdup

#define strlen              strlen

#define sprintf             sprintf
#define _vsntprintf         vsnprintf
#define _sntprintf          snprintf

#define _tprintf            printf
#define _T(x)               x

#define _istalnum           isalnum

#define _tcsftime           strftime

#define MAX_PATH            260

/////////////////////////////////////////////////////////////////////////////
// cSize - An extent, similar to Windows SIZE structure.

class CPoint;
class CRect;

typedef struct  tagSIZE
{
    int cx;
    int cy;
}SIZE, *LPSIZE;

#ifdef _LINUX_GTK2
typedef GdkPoint tagPOINT;
typedef GdkPoint CPoint;
typedef GdkPoint *LPPOINT;
#else // _LINUX_GTK2
typedef struct  tagPOINT
{
    int x;
    int y;
}CPoint, *LPPOINT;
#endif // _LINUX_GTK2

typedef struct  tagRECT
{
    int left;
    int top;
    int right;
    int bottom;
}CRect, *LPRECT;

class cSize : public tagSIZE {
public:

    // Constructors
    // construct an uninitialized size
    cSize();
    // create from two integers
    cSize(int initCX, int initCY);
    // create from another size
    cSize(SIZE initSize);
    // create from a point
    cSize(CPoint initPt);
    // create from a uint32_t: cx = LOWORD(dw) cy = HIWORD(dw)
    cSize(uint32_t dwSize);

    // Operations
    bool operator==(SIZE size) const;
    bool operator!=(SIZE size) const;
    void operator+=(SIZE size);
    void operator-=(SIZE size);

    // Operators returning cSize values
    cSize operator+(SIZE size) const;
    cSize operator-(SIZE size) const;
    cSize operator-() const;

    // Operators returning CPoint values
    CPoint operator+(CPoint point) const;
    CPoint operator-(CPoint point) const;

    // Operators returning CRect values
    CRect operator+(const CRect* lpRect) const;
    CRect operator-(const CRect* lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows CPoint structure.

class CPoint : public tagPOINT {
public:
    // Constructors

    // create an uninitialized point
    CPoint();
    // create from two integers
    CPoint(int initX, int initY);
    // create from another point
    CPoint(CPoint initPt);
    // create from a size
    CPoint(SIZE initSize);
    // create from a dword: x = LOWORD(dw) y = HIWORD(dw)
    CPoint(uint32_t dwPoint);

    // Operations

    // translate the point
    void offset(int xOffset, int yOffset);
    void offset(CPoint point);
    void offset(SIZE size);

    bool operator==(CPoint point) const;
    bool operator!=(CPoint point) const;
    void operator+=(SIZE size);
    void operator-=(SIZE size);
    void operator+=(CPoint point);
    void operator-=(CPoint point);

    // Operators returning CPoint values
    CPoint operator+(SIZE size) const;
    CPoint operator-(SIZE size) const;
    CPoint operator-() const;
    CPoint operator+(CPoint point) const;

    // Operators returning cSize values
    cSize operator-(CPoint point) const;

    // Operators returning CRect values
    CRect operator+(const CRect* lpRect) const;
    CRect operator-(const CRect* lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows CRect structure.

typedef const CRect* LPCRECT; // pointer to read/only CRect

class CRect : public tagRECT {
public:

    // Constructors

    // uninitialized rectangle
    CRect();
    // from left, top, right, and bottom
    CRect(int l, int t, int r, int b);
    // copy constructor
    CRect(const CRect& srcRect);
    // from a pointer to another rect
    CRect(CRect *lpSrcRect);
    // from a point and size
    CRect(CPoint point, SIZE size);
    // from two points
    CRect(CPoint topLeft, CPoint bottomRight);

    // Attributes (in addition to CRect members)

    // retrieves the width
    int width() const;
    // returns the height
    int height() const;
    // returns the size
    cSize size() const;
    // reference to the top-left point
    CPoint& TopLeft();
    // reference to the bottom-right point
    CPoint& BottomRight();
    // const reference to the top-left point
    const CPoint& TopLeft() const;
    // const reference to the bottom-right point
    const CPoint& BottomRight() const;
    // the geometric center point of the rectangle
    CPoint centerPoint() const;
    // swap the left and right
    void swapLeftRight();
    static void swapLeftRight(CRect &rcPos);

    // convert between CRect and LPRECT/CRect *(no need for &)
    operator LPRECT();
    operator LPCRECT() const;

    // returns true if rectangle has no area
    bool isRectEmpty() const;
    // returns true if rectangle is at (0,0) and has no area
    bool isRectNull() const;
    // returns true if point is within rectangle
    bool ptInRect(CPoint point) const;

    // Operations

    // set rectangle from left, top, right, and bottom
    void setLTRB(int x1, int y1, int x2, int y2);
    void setLTRB(CPoint topLeft, CPoint bottomRight);
    // empty the rectangle
    void setRectEmpty();
    // copy from another rectangle
    void copyRect(CRect *lpSrcRect);
    // true if exactly the same as another rectangle
    bool equalRect(CRect *lpRect) const;

    // inflate rectangle's width and height without
    // moving its top or left
    void inflateRect(int x, int y);
    void inflateRect(SIZE size);
    void inflateRect(CRect *lpRect);
    //    void inflateRect(int l, int t, int r, int b);
    // deflate the rectangle's width and height without
    // moving its top or left
    void deflateRect(int x, int y);
    void deflateRect(SIZE size);
    void deflateRect(CRect *lpRect);
    //    void deflateRect(int l, int t, int r, int b);

    // translate the rectangle by moving its top and left
    void offsetRect(int x, int y);
    void offsetRect(SIZE size);
    void offsetRect(CPoint point);
    //    void NormalizeRect();

    // set this rectangle to intersection of two others
    bool intersectRect(CRect *lpRect1, CRect *lpRect2);

    // set this rectangle to bounding union of two others
    bool unionRect(CRect *lpRect1, CRect *lpRect2);

    // set this rectangle to minimum of two others
    //    bool subtractRect(CRect *lpRectSrc1, CRect *lpRectSrc2);

    // Additional Operations
    void operator=(const CRect& srcRect);
    bool operator==(const CRect& rect) const;
    bool operator!=(const CRect& rect) const;
    void operator+=(CPoint point);
    void operator+=(SIZE size);
    //    void operator+=(CRect *lpRect);
    void operator-=(CPoint point);
    void operator-=(SIZE size);
    //    void operator-=(CRect *lpRect);
    //    void operator&=(const CRect& rect);
    //    void operator|=(const CRect& rect);

    // Operators returning CRect values
    CRect operator+(CPoint point) const;
    CRect operator-(CPoint point) const;
    CRect operator+(CRect *lpRect) const;
    CRect operator+(SIZE size) const;
    CRect operator-(SIZE size) const;
    CRect operator-(CRect *lpRect) const;
    //    CRect operator&(const CRect& rect2) const;
    //    CRect operator|(const CRect& rect2) const;
    //    CRect MulDiv(int nMultiplier, int nDivisor) const;
};

inline bool isRectEmpty(CRect *lpRect)
    { return lpRect->left >= lpRect->right || lpRect->top >= lpRect->bottom; }

inline cSize::cSize()
    {  }
inline cSize::cSize(int initCX, int initCY)
    { cx = initCX; cy = initCY; }
inline cSize::cSize(SIZE initSize)
    { *(SIZE*)this = initSize; }
inline cSize::cSize(CPoint initPt)
    { *(CPoint*)this = initPt; }
inline cSize::cSize(uint32_t dwSize) {
    cx = (short)LOWORD(dwSize);
    cy = (short)HIWORD(dwSize);
}
inline bool cSize::operator==(SIZE size) const
    { return (cx == size.cx && cy == size.cy); }
inline bool cSize::operator!=(SIZE size) const
    { return (cx != size.cx || cy != size.cy); }
inline void cSize::operator+=(SIZE size)
    { cx += size.cx; cy += size.cy; }
inline void cSize::operator-=(SIZE size)
    { cx -= size.cx; cy -= size.cy; }
inline cSize cSize::operator+(SIZE size) const
    { return cSize(cx + size.cx, cy + size.cy); }
inline cSize cSize::operator-(SIZE size) const
    { return cSize(cx - size.cx, cy - size.cy); }
inline cSize cSize::operator-() const
    { return cSize(-cx, -cy); }
inline CPoint cSize::operator+(CPoint point) const
    { return CPoint(cx + point.x, cy + point.y); }
inline CPoint cSize::operator-(CPoint point) const
    { return CPoint(cx - point.x, cy - point.y); }
inline CRect cSize::operator+(const CRect* lpRect) const
    { return CRect(lpRect) + *this; }
inline CRect cSize::operator-(const CRect* lpRect) const
    { return CRect(lpRect) - *this; }

// CPoint
inline CPoint::CPoint()
    { }
inline CPoint::CPoint(int initX, int initY)
    { x = initX; y = initY; }
inline CPoint::CPoint(CPoint initPt)
    { x = initPt.x; y = initPt.y; }
inline CPoint::CPoint(SIZE initSize)
    { *(SIZE*)this = initSize; }
inline CPoint::CPoint(uint32_t dwPoint) {
    x = (short)LOWORD(dwPoint);
    y = (short)HIWORD(dwPoint);
}
inline void CPoint::offset(int xOffset, int yOffset)
    { x += xOffset; y += yOffset; }
inline void CPoint::offset(CPoint point)
    { x += point.x; y += point.y; }
inline void CPoint::offset(SIZE size)
    { x += size.cx; y += size.cy; }
inline bool CPoint::operator==(CPoint point) const
    { return (x == point.x && y == point.y); }
inline bool CPoint::operator!=(CPoint point) const
    { return (x != point.x || y != point.y); }
inline void CPoint::operator+=(SIZE size)
    { x += size.cx; y += size.cy; }
inline void CPoint::operator-=(SIZE size)
    { x -= size.cx; y -= size.cy; }
inline void CPoint::operator+=(CPoint point)
    { x += point.x; y += point.y; }
inline void CPoint::operator-=(CPoint point)
    { x -= point.x; y -= point.y; }
inline CPoint CPoint::operator+(SIZE size) const
    { return CPoint(x + size.cx, y + size.cy); }
inline CPoint CPoint::operator-(SIZE size) const
    { return CPoint(x - size.cx, y - size.cy); }
inline CPoint CPoint::operator-() const
    { return CPoint(-x, -y); }
inline CPoint CPoint::operator+(CPoint point) const
    { return CPoint(x + point.x, y + point.y); }
inline cSize CPoint::operator-(CPoint point) const
    { return cSize(x - point.x, y - point.y); }
inline CRect CPoint::operator+(const CRect* lpRect) const
    { return CRect(lpRect) + *this; }
inline CRect CPoint::operator-(const CRect* lpRect) const
    { return CRect(lpRect) - *this; }

// CRect
inline CRect::CRect()
    {  }
inline CRect::CRect(int l, int t, int r, int b)
    { left = l; top = t; right = r; bottom = b; }
inline CRect::CRect(const CRect& srcRect)
    { copyRect(&srcRect); }
inline CRect::CRect(CRect *lpSrcRect)
    { copyRect(lpSrcRect); }
inline CRect::CRect(CPoint point, SIZE size)
    { right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }
inline CRect::CRect(CPoint topLeft, CPoint bottomRight)
    { left = topLeft.x; top = topLeft.y; right = bottomRight.x; bottom = bottomRight.y; }
inline int CRect::width() const
    { return (int)(right - left); }
inline int CRect::height() const
    { return (int)(bottom - top); }
inline cSize CRect::size() const
    { return cSize(right - left, bottom - top); }
inline CPoint& CRect::TopLeft()
    { return *((CPoint*)this); }
inline CPoint& CRect::BottomRight()
    { return *((CPoint*)this+1); }
inline const CPoint& CRect::TopLeft() const
    { return *((CPoint*)this); }
inline const CPoint& CRect::BottomRight() const
    { return *((CPoint*)this+1); }
inline CPoint CRect::centerPoint() const
    { return CPoint((left+right)/2, (top+bottom)/2); }
inline void CRect::swapLeftRight()
    { swapLeftRight(LPRECT(this)); }
inline void CRect::swapLeftRight(CRect &rcPos)
    { int temp = lpRect->left; lpRect->left = lpRect->right; lpRect->right = temp; }
inline CRect::operator LPRECT()
    { return this; }
inline CRect::operator LPCRECT() const
    { return this; }
inline bool CRect::isRectEmpty() const
    { return left >= right || top >= bottom; }
inline bool CRect::isRectNull() const
    { return (left == 0 && right == 0 && top == 0 && bottom == 0); }
inline bool CRect::ptInRect(CPoint point) const
    { return point.x >= left && point.x < right && point.y >= top && point.y < bottom; }
inline void CRect::setLTRB(int x1, int y1, int x2, int y2)
    { left = x1; top = y1; right = x2; bottom = y2; }
inline void CRect::setLTRB(CPoint topLeft, CPoint bottomRight)
    { setLTRB(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
inline void CRect::setRectEmpty()
    { left = right = top = bottom = 0; }
inline void CRect::copyRect(CRect *lpSrcRect) {
    left = lpSrcRect->left; top = lpSrcRect->top;
    right = lpSrcRect->right; bottom = lpSrcRect->bottom;
}
inline bool CRect::equalRect(CRect *lpRect) const
    { return left == lpRect->left && top == lpRect->top && right == lpRect->right && bottom == lpRect->bottom; }
inline void CRect::inflateRect(int x, int y)
    { left -= x; right += x; top -= y; bottom += y; }
inline void CRect::inflateRect(SIZE size)
    { inflateRect((int)size.cx, (int)size.cy); }
inline void CRect::deflateRect(int x, int y)
    { inflateRect(-x, -y); }
inline void CRect::deflateRect(SIZE size)
    { inflateRect((int)(-size.cx), (int)(-size.cy)); }
inline void CRect::offsetRect(int x, int y)
    { left += x; right += x; top += y; bottom += y; }
inline void CRect::offsetRect(CPoint point)
    { offsetRect((int)point.x, (int)point.y); }
inline void CRect::offsetRect(SIZE size)
    { offsetRect((int)size.cx, (int)size.cy); }
inline bool CRect::intersectRect(CRect *lpRect1, CRect *lpRect2) {
    if (::isRectEmpty(lpRect1) || ::isRectEmpty(lpRect2) ||
        (lpRect1->left >= lpRect2->right) || (lpRect2->left >= lpRect1->right) ||
        (lpRect1->top >= lpRect2->bottom) || (lpRect2->top >= lpRect1->bottom)) {
        setRectEmpty();
        return false;
    }
    left = max(lpRect1->left, lpRect2->left);
    right = min(lpRect1->right, lpRect2->right);
    top = max(lpRect1->top, lpRect2->top);
    bottom = min(lpRect1->bottom, lpRect2->bottom);
    return true;
}
inline bool CRect::unionRect(CRect *lpRect1, CRect *lpRect2) {
    if (::isRectEmpty(lpRect1)) {
        if (::isRectEmpty(lpRect2)) {
            setRectEmpty();
            return false;
        } else {
            *this = *lpRect2;
        }
    } else {
        if (::isRectEmpty(lpRect2)) {
            *this = *lpRect1;
        } else {
            left = min(lpRect1->left, lpRect2->left);
            right = max(lpRect1->right, lpRect2->right);
            top = min(lpRect1->top, lpRect2->top);
            bottom = max(lpRect1->bottom, lpRect2->bottom);
        }
    }
    return true;
}
inline void CRect::operator=(const CRect& srcRect)
    { copyRect(&srcRect); }
inline bool CRect::operator==(const CRect& rect) const
    { return equalRect(&rect); }
inline bool CRect::operator!=(const CRect& rect) const
    { return !equalRect(&rect); }
inline void CRect::operator+=(CPoint point)
    { offsetRect((int)point.x, (int)point.y); }
inline void CRect::operator+=(SIZE size)
    { offsetRect((int)size.cx, (int)size.cy); }
/*inline void CRect::operator+=(CRect *lpRect)
    {
        left += lpRect->left;
        right += lpRect->right;
        top += lpRect->top;
        bottom += lpRect->bottom;
    }*/
inline void CRect::operator-=(CPoint point)
    { offsetRect((int)(-point.x), (int)(-point.y)); }
inline void CRect::operator-=(SIZE size)
    { offsetRect((int)(-size.cx), (int)(-size.cy)); }
//inline void CRect::operator-=(CRect *lpRect)
//    {
//        left -= lpRect->left;
//        right -= lpRect->right;
//        top -= lpRect->top;
//        bottom -= lpRect->bottom;
//    }
//inline void CRect::operator&=(const CRect& rect)
//    { ::intersectRect(this, this, &rect); }
//inline void CRect::operator|=(const CRect& rect)
//    { ::unionRect(this, this, &rect); }
inline CRect CRect::operator+(CPoint pt) const
    { CRect rect(*this); rect.offsetRect((int)pt.x, (int)pt.y); return rect; }
inline CRect CRect::operator-(CPoint pt) const
    { CRect rect(*this); rect.offsetRect((int)-pt.x, (int)-pt.y); return rect; }
inline CRect CRect::operator+(SIZE size) const
    { CRect rect(*this); rect.offsetRect((int)size.cx, (int)size.cy); return rect; }
inline CRect CRect::operator-(SIZE size) const
    { CRect rect(*this); rect.offsetRect((int)-size.cx, (int)-size.cy); return rect; }
//inline CRect CRect::operator+(CRect *lpRect) const
//    { CRect rect(this); rect.inflateRect(lpRect); return rect; }
//inline CRect CRect::operator-(CRect *lpRect) const
//    { CRect rect(this); rect.deflateRect(lpRect); return rect; }
//inline CRect CRect::operator&(const CRect& rect2) const
//    { CRect rect; ::intersectRect(&rect, this, &rect2);
//        return rect; }
//inline CRect CRect::operator|(const CRect& rect2) const
//    { CRect rect; ::unionRect(&rect, this, &rect2);
//        return rect; }
//inline bool CRect::subtractRect(CRect *lpRectSrc1, CRect *lpRectSrc2)
//    { return ::subtractRect(this, lpRectSrc1, lpRectSrc2); }

#endif

#endif // _WIN_TYPE_H
