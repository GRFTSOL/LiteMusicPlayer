#include "../Cursor.h"


void setCursor(Cursor &cursor) {
    ::setCursor(Cursor.m_cursor);
}

Cursor::Cursor(void) {
    m_cursor = nullptr;
}

Cursor::~Cursor(void) {
    destroy();
}

bool Cursor::isValid() const {
    return m_cursor != nullptr;
}

bool Cursor::loadStdCursor(STD_CURSOR_TYPE cusorType) {
    m_cursor = ::loadCursor(nullptr, MAKEINTRESOURCE(cusorType));
    return m_cursor != nullptr;
}

bool Cursor::loadCursorFromFile(cstr_t szFile) {
    m_cursor = ::loadCursorFromFile(szFile);
    return m_cursor != nullptr;
}

void Cursor::destroy() {
    if (m_cursor) {
        m_cursor = nullptr;
    }
}
