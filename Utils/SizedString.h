//
//  Created by HongyongXiao on 2021/11/12.
//

#ifndef SizedString_hpp
#define SizedString_hpp

#pragma once

#include <cstddef>
#include <stdint.h>
#include <memory.h>
#include <string>

using namespace std;

inline bool isEmptyString(const char *s) { return s == nullptr || s[0] == '\0'; }
inline void emptyStr(char *s) { s[0] = '\0'; }

#define isSpace isspace


class SizedString {
public:
    SizedString() { data = nullptr; len = 0; }
    SizedString(const char *data);
    SizedString(const string s) : data((uint8_t *)s.c_str()), len(s.size()) { }
    SizedString(const void *data, size_t len) : data((const uint8_t *)data), len(len) { }
    SizedString(size_t len, const void *data) : data((const uint8_t *)data), len(len) { }
    SizedString(const uint8_t *data, size_t len) : data(data), len(len) { }

    uint8_t *strlchr(uint8_t c) const;
    uint8_t *strrchr(uint8_t c) const;

    int strStr(const SizedString &find) const;
    int strIStr(const SizedString &find) const;

    int cmp(const SizedString &other) const;
    int iCmp(const SizedString &other) const;

    inline bool equal(const char *other) const {
        return cmp(SizedString(other)) == 0;
    }

    inline bool equal(const SizedString &other) const {
        return cmp(other) == 0;
    }

    inline bool iEqual(const SizedString &other) const {
        return iCmp(other) == 0;
    }

    bool startsWith(const SizedString &with) const;
    bool iStartsWith(const SizedString &with) const;

    inline bool endsWith(const SizedString &with) const {
        if (len >= with.len) {
            return memcmp(data + len - with.len, with.data, with.len) == 0;
        }
        return false;
    }

    void trim(uint8_t charToTrim);
    void trim(const SizedString &toTrim);

    void shrink(int startShrinkSize, int endShrinkSize = 0);
    inline void shrink(size_t startShrinkSize, size_t endShrinkSize = 0) {
        shrink((int)startShrinkSize, (int)endShrinkSize);
    }

    SizedString subStr(size_t offset, size_t size) const;

    long atoi(const SizedString &s) const;
    SizedString itoa(long num, char *str) const;

    void toLowerCase();

    template <class _Container>
    void split(char chSeparator, _Container &container, int count = -1) {
        if (len == 0) {
            return;
        }

        const uint8_t *p, *start, *end;

        start = p = data;
        end = data + len;
        while (p < end && container.size() + 1 < (unsigned int)count) {
            while (p < end && *p != chSeparator) {
                p++;
            }

            if (p < end) {
                typename _Container::value_type tmp((const char *)start, size_t(p - start));
                container.push_back(tmp);
                p++;
            } else {
                typename _Container::value_type tmp((const char *)start, size_t(end - start));
                container.push_back(tmp);
                return;
            }

            start = p;
        }

        typename _Container::value_type tmp((const char *)start, size_t(end - start));
        container.push_back(tmp);
    }

    bool split(char chSeparator, SizedString &left, SizedString &right);
    bool split(const char *separator, SizedString &left, SizedString &right);

public:
    const uint8_t                   *data;
    size_t                          len;

};


#define MAKE_SIZED_STR(s)       SizedString(s, sizeof(s) - 1)

template<typename STR>
inline SizedString makeSizedString(const STR &s) { return SizedString(s); }

inline string makeString(const SizedString &str) { return string((const char *)str.data, str.len); }

#define SIZE_STR_NULL           SizedString(0, (const char *)nullptr);

struct SizedStrCmpLess {

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) < 0;
    }

};

struct NgxStrEqualCmp {
    bool operator()(const SizedString &first, const SizedString &other) const {
        return first.equal(other);
    }
};

bool strIsInList(SizedString &str, SizedString *arr, size_t count);
bool IStrIsInList(SizedString &str, SizedString *arr, size_t count);

#endif /* SizedString_hpp */
