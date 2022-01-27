#ifndef _STRING_ITERATOR_H_
#define _STRING_ITERATOR_H_

#pragma once


class Utf8CharType
{
protected:
    char        szChar[4];

public:
    Utf8CharType(cstr_t str) {
        set(str);
    }

    Utf8CharType(char ch) {
        szChar[0] = ch;
        szChar[1] = 0;
    }

    Utf8CharType() { szChar[0] = 0; }

    void operator + (const char *str) { set(str); }

    bool operator == (Utf8CharType ch) {
        for (int i = 0; szChar[i] != 0; i++) {
            if (szChar[i] != ch.szChar[i])
                return false;
        }
        return true;
    }

    bool operator != (Utf8CharType ch) {
        return !(*this == ch);
    }

    void set(const char *str) {
        unsigned char    firstChar = (unsigned char)(str[0]);
        int                n;

        if (firstChar < 0xc0)            n = 1;
        else if (firstChar < 0xe0)        n = 2;
        else if (firstChar < 0xf0)        n = 3;
        else                            n = 1;

        for (int i = 0; i < n; i++)
            szChar[i] = str[i];
        szChar[n] = 0;
    }

    int32_t getIntValue() {
        uint32_t        value = 0;

        for (int i = 0; szChar[i] != 0; i++)
        {
            value <<= 8;
            value |= uint8_t(szChar[i]);
        }

        return value;
    }

    operator const char *() { return szChar; }

    size_t size() { return strlen(szChar); }

};

template<class _CharType, class _char_t>
class StringIterator_t
{
public:
    typedef _CharType        CharType;

    StringIterator_t(const _char_t *szText) {
        m_szText = szText;
        m_nPos = 0;
        m_ch = szText;
    }

    void operator = (const _char_t *szText) {
        m_szText = szText;
        m_nPos = 0;
        m_ch = szText;
    }

    // Is end of string
    bool isEOS() { return m_szText[m_nPos] == 0; }

    CharType &curChar() {
        return m_ch;
    }

    void operator ++() {
        m_nPos += m_ch.size();
        m_ch = m_szText + m_nPos;
    }

    int getPos() { return m_nPos; }
    void setPos(int nPos) { m_nPos = nPos; m_ch = m_szText + m_nPos; }

protected:
    const _char_t       *m_szText;
    int                 m_nPos;
    CharType            m_ch;

};

typedef StringIterator_t<Utf8CharType, char>        StringIterator;


#endif // _STRING_ITERATOR_H_
