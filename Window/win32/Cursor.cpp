#include "../WindowLib.h"
#include "../Cursor.h"


void setCursor(Cursor &cursor) {
    ::SetCursor(cursor.getHandle());
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
    m_cursor = ::LoadCursor(nullptr, MAKEINTRESOURCE(cusorType));
    return m_cursor != nullptr;
}

bool Cursor::loadCursorFromFile(cstr_t szFile) {
    m_cursor = ::LoadCursorFromFileW(utf8ToUCS2(szFile).c_str());
    return m_cursor != nullptr;
}

void Cursor::destroy() {
    if (m_cursor) {
        m_cursor = nullptr;
    }
}

void Cursor::set() {
    ::SetCursor(m_cursor);
}
