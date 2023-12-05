#import <AppKit/AppKit.h>
#include "../WindowTypes.h"
#include "../Cursor.h"


struct _CursorInternal {
    NSCursor                *cursor = nullptr;
};

void setCursor(Cursor &cursor) {
    cursor.set();
}

Cursor::Cursor(void) {
    _data = new _CursorInternal;
}

Cursor::~Cursor(void) {
    destroy();
}

bool Cursor::isValid() const {
    return _data && _data->cursor;
}

bool Cursor::loadStdCursor(STD_CURSOR_TYPE cursorType) {
    NSCursor *c = nullptr;
    switch (cursorType) {
        case C_ARROW:
            c = [NSCursor arrowCursor];
            break;
        case C_CROSS:
            c = [NSCursor crosshairCursor];
            break;
        case C_SIZENESW:
            break;
        case C_SIZENS:
            c = [NSCursor resizeUpDownCursor];
            break;
        case C_SIZENWSE:
            break;
        case C_SIZEWE:
            c = [NSCursor resizeLeftRightCursor];
            break;
        case C_NO:
            c = [NSCursor operationNotAllowedCursor];
            break;
        case C_IBEAM:
            c = [NSCursor IBeamCursor];
            break;
        case C_HAND:
            c = [NSCursor openHandCursor];
            break;
        default:
            assert(0);
            break;
    }

    if (_data->cursor) {
        [_data->cursor release];
    }
    _data->cursor = c;

    if (c) {
        [c retain];
    }

    return c != nullptr;
}

bool Cursor::loadCursorFromFile(cstr_t szFile) {
    // return m_cursor != nullptr;
    return false;
}

void Cursor::destroy() {
    if (_data) {
        if (_data->cursor) {
            [_data->cursor release];
        }
        delete _data;
    }
}

void Cursor::set() {
    if (_data && _data->cursor) {
        [_data->cursor set];
    }
}
