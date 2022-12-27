#include "WindowTypes.h"
#include "Cursor.h"


Cursor::Cursor(void) {
    m_cursor = nullptr;
}

Cursor::~Cursor(void) {
    destroy();
}

bool Cursor::loadStdCursor(STD_CURSOR_TYPE cusorType) {
    return m_cursor != nullptr;
}

bool Cursor::loadCursorFromFile(cstr_t szFile) {
    return m_cursor != nullptr;
}

bool Cursor::loadCursorFromRes(int nID) {
    return m_cursor != nullptr;
}

void Cursor::destroy() {
    if (m_cursor) {
        m_cursor = nullptr;
    }
}
