#pragma once

#include "../Utils/Utils.h"
#include "../GfxRaw/GfxRawTypes.h"


struct CPoint {
    int                         x;
    int                         y;

    CPoint() : x(0), y(0) { }
    CPoint(int x, int y) : x(x), y(y) { }

    bool operator == (const CPoint &other) const {
        return x == other.x && y == other.y;
    }

};

using VecPoints = std::vector<CPoint>;

struct CSize {
    int                         cx;
    int                         cy;

    CSize() : cx(0), cy(0) { }
    CSize(int cx, int cy) : cx(cx), cy(cy) { }

    bool operator == (const CSize &other) const {
        return cx == other.cx && cy == other.cy;
    }

};

struct CRect {
    int                         left;
    int                         top;
    int                         right;
    int                         bottom;

    CRect() : left(0), top(0), right(0), bottom(0) { }
    CRect(int left, int top, int right, int bottom) : left(left), top(top), right(right), bottom(bottom) { }

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

    bool operator == (const CRect &other) const {
        return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
    }

    bool operator != (const CRect &other) const {
        return left != other.left || top != other.top || right != other.right || bottom != other.bottom;
    }

};

inline CRect makeRectLTWH(int left, int top, int width, int height)
    { return CRect(left, top, left + width, top + height); }
