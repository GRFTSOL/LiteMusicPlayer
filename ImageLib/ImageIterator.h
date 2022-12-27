#pragma once

#ifndef ImageLib_ImageIterator_h
#define ImageLib_ImageIterator_h


class CImageIterator {
protected:
    int                         m_xbyte, m_y;       // Counters
    int                         m_stepx, m_stepy;
    uint8_t*        m_pRow;
    RawImageData                *m_pimage;

public:
    // Constructors
    CImageIterator(void) {
        m_xbyte = 0;
        m_y = 0;
        m_stepx = 0;
        m_stepy = 0;
        m_pimage = nullptr;
        m_pRow = nullptr;
    }
    CImageIterator(RawImageData *img) {
        m_xbyte = 0;
        m_y = 0;
        m_stepx = 0;
        m_stepy = 0;
        m_pimage = img;
        m_pRow = m_pimage->rowPtr(0);
    }
    operator RawImageData *() {
        return m_pimage;
    }

    // Iterators
    bool itOK() {
        if (m_pimage && m_pimage->buff
            && m_xbyte >= 0 && m_xbyte < m_pimage->width
            && m_y >= 0 && m_y < m_pimage->height) {
            return true;
        }
        return false;
    }
    void reset() {
        m_xbyte = 0;
        m_y = 0;
        if (m_pimage) {
            m_pRow = m_pimage->rowPtr(0);
        } else {
            m_pRow = nullptr;
        }
    }
    void upset() {
        m_xbyte = 0;
        if (m_pimage) {
            m_y = m_pimage->height - 1;
            m_pRow = m_pimage->rowPtr(m_y);
        } else {
            m_y = 0;
            m_pRow = nullptr;
        }
    }
    void setRow(uint8_t *buf, int n) {
        if (n < 0) {
            n = m_pimage->absStride();
        } else {
            n = min(n, (int)m_pimage->absStride());
        }

        if (m_pRow) {
            memcpy(m_pRow, buf, n);
        }
    }
    void getRow(uint8_t *buf, int n) {
        assert(n <= m_pimage->absStride());
        if (buf && n > 0 && n <= m_pimage->absStride()) {
            memcpy(buf, m_pRow, n);
        }
    }
    uint8_t getByte( ) { return m_pRow[m_xbyte]; }
    void setByte(uint8_t b) { m_pRow[m_xbyte] = b; }
    uint8_t* getRow(void) { return m_pRow; }
    uint8_t* getRow(int nRow) {
        setY(nRow);
        return m_pRow;
    }
    bool nextRow() {
        m_y++;
        if (m_y >= m_pimage->height) {
            return false;
        }
        m_pRow += m_pimage->stride;

        return true;
    }
    bool prevRow() {
        m_y--;
        if (m_y < 0) {
            return false;
        }
        m_pRow -= m_pimage->stride;

        return true;
    }
    bool nextByte() {
        m_xbyte++;
        if (m_xbyte < (int)m_pimage->absStride()) {
            return true;
        } else {
            m_y++;
            if (m_y < (int)m_pimage->height) {
                m_pRow += m_pimage->stride;
                m_xbyte = 0;
                return true;
            } else {
                return false;
            }
        }
    }
    bool prevByte() {
        m_xbyte--;
        if (m_xbyte >= 0) {
            return true;
        } else {
            m_y--;
            if (m_y >= 0 && m_y < (int)m_pimage->height) {
                m_pRow -= m_pimage->stride;
                m_xbyte = 0;
                return true;
            } else {
                return false;
            }
        }
    }

    void setSteps(int x, int y=0) {  m_stepx = x; m_stepy = y; }
    void getSteps(int &x, int &y) {  x = m_stepx; y = m_stepy; }
    bool nextStep() {
        m_xbyte += m_stepx;
        if (m_xbyte < (int)m_pimage->absStride()) {
            return true;
        } else {
            m_y += m_stepy;
            if (m_y < (int)m_pimage->height) {
                m_pRow += m_pimage->stride;
                m_xbyte = 0;
                return true;
            } else {
                return false;
            }
        }
    }
    bool prevStep() {
        m_xbyte -= m_stepx;
        if (m_xbyte >= 0) {
            return true;
        } else {
            m_y -= m_stepy;
            if (m_y >= 0 && m_y < (int)m_pimage->height) {
                m_pRow -= m_pimage->stride;
                m_xbyte = 0;
                return true;
            } else {
                return false;
            }
        }
    }
    void setY(int y) {
        if (y < 0 || y >= m_pimage->height) {
            return;
        }
        m_y = y;
        m_pRow = m_pimage->rowPtr(y);
    }
    int  getY() { return m_y; }
};

#endif // !defined(ImageLib_ImageIterator_h)
