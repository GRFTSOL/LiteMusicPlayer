//
//  Created by HongyongXiao on 2021/11/12.
//

#include "UtilsTypes.h"
#include "SizedString.h"
#include <ctype.h>
#include <strings.h>
#include <climits>


SizedString::SizedString(const char *data) : data((const uint8_t *)data), len(strlen((const char *)data)) {
}

uint8_t *SizedString::strlchr(uint8_t c) const {
    const uint8_t *p = data, *last = data + len;

    while (p < last) {
        if (*p == c) {
            return (uint8_t *)p;
        }

        p++;
    }

    return nullptr;
}

uint8_t *SizedString::strrchr(uint8_t c) const {
    const uint8_t *start = data, *p = data + len;

    if (len == 0) {
        return nullptr;
    }

    while (p >= start) {
        if (*p == c) {
            return (uint8_t *)p;
        }

        p--;
    }

    return nullptr;
}

int SizedString::strStr(const SizedString &find) const {
    if (len < find.len) {
        return -1;
    }

    const uint8_t *last = data + len - find.len;
    const uint8_t *s1 = data, *s2 = find.data;
    size_t len = find.len - 1;
    uint8_t  c1, c2;

    c2 = (uint8_t)*s2++;

    do {
        do {
            if (s1 > last) {
                return -1;
            }

            c1 = *s1++;
        } while (c1 != c2);

    } while (memcmp(s1, s2, len) != 0);

    return (int)(s1 - data - 1);
}

int SizedString::strIStr(const SizedString &find) const {
    if (len < find.len) {
        return -1;
    }

    const uint8_t *last = data + len - find.len;
    const uint8_t *s1 = data, *s2 = find.data;
    size_t len = find.len - 1;
    uint8_t  c1, c2;

    c2 = (uint8_t)*s2++;

    do {
        do {
            if (s1 > last) {
                return -1;
            }

            c1 = *s1++;
        } while (tolower((int)c1) != tolower((int)c2));

    } while (strncasecmp((char *)s1, (char *)s2, len) != 0);

    return (int)(s1 - data - 1);
}

int SizedString::cmp(const SizedString &other) const {
    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = other.data, *p2End = other.data + other.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        if (*p1 == *p2) {
            continue;
        }
        return *p1 - *p2;
    }

    if (p2 == p2End) {
        if (p1 == p1End) {
            return 0;
        }
        return 1;
    } else {
        return -1;
    }
}

int SizedString::iCmp(const SizedString &other) const {
    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = other.data, *p2End = other.data + other.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        uint8_t c1 = *p1, c2 = *p2;

        if ('A' <= c1 && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if ('A' <= c2 && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 == c2) {
            continue;
        }
        return c1 - c2;
    }

    if (p2 == p2End) {
        if (p1 == p1End) {
            return 0;
        }
        return 1;
    } else {
        return -1;
    }
}

bool SizedString::startsWith(const SizedString &with) const {
    if (len < with.len) {
        return false;
    }

    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = with.data, *p2End = with.data + with.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        if (*p1 == *p2) {
            continue;
        }
        return false;
    }

    return p2 == p2End;
}

bool SizedString::iStartsWith(const SizedString &with) const {
    if (len < with.len) {
        return false;
    }

    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = with.data, *p2End = with.data + with.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        uint8_t c1 = *p1, c2 = *p2;

        if ('A' <= c1 && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if ('A' <= c2 && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 == c2) {
            continue;
        }
        return false;
    }

    return p2 == p2End;
}

void SizedString::trim(uint8_t charToTrim) {
    const uint8_t *end = data + len;
    const uint8_t *p = data;

    while (p < end && *p == (uint8_t)charToTrim) {
        p++;
    }

    while (end > p && *(end - 1) == (uint8_t)charToTrim) {
        end--;
    }

    len -= (p - data);
    data = p;
}

void SizedString::trim(const SizedString &toTrim) {
    // Trim from tail
    const uint8_t *start = data, *end = data + len;
    while (start < end) {
        if (toTrim.strlchr(*(end - 1)) == nullptr) {
            break;
        }
        --end;
    }

    // Trim from head
    while (start < end) {
        if (toTrim.strlchr(*(start)) == nullptr) {
            break;
        }
        ++start;
    }

    data = start;
    len = (size_t)(end - start);
}


void SizedString::shrink(int startShrinkSize, int endShrinkSize) {
    data += startShrinkSize;
    if (startShrinkSize + endShrinkSize >= (int)len) {
        len = 0;
    } else {
        len -= startShrinkSize + endShrinkSize;
    }
}

SizedString SizedString::subStr(size_t offset, size_t size) const {
    if (offset + size <= len) {
        return SizedString(size, data + offset);
    } else if (offset <= len) {
        return SizedString(len - offset, data + offset);
    }

    return SIZE_STR_NULL;
}

long SizedString::atoi(const SizedString &s) const {
    long value, cutoff, cutlim;

    if (len == 0) {
        return -1;
    }

    cutoff = LONG_MAX / 10;
    cutlim = LONG_MAX % 10;

    int n = (int)len;
    const uint8_t *p = data;
    for (value = 0; n--; p++) {
        if (*p < '0' || *p > '9') {
            return -1;
        }

        if (value >= cutoff && (value > cutoff || *p - '0' > cutlim)) {
            return -1;
        }

        value = value * 10 + (*p - '0');
    }

    return value;
}

void reverse(char str[], size_t length) {
    char *p = str, *end = str + length - 1;
    while (p < end) {
        char tmp = *p;
        *p = *end;
        *end = tmp;

        p++;
        end--;
    }
}

SizedString SizedString::itoa(long num, char *str) const {
    size_t i = 0;
    bool isNegative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return {i, (uint8_t *)str};
    }

    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    // append '-'
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return {i, (uint8_t *)str};
}

void SizedString::toLowerCase() {
    uint8_t *p = (uint8_t *)data, *last = (uint8_t *)data + len;

    while (p < last) {
        if ('A' <= *p && *p <= 'Z') {
            *p += 'a' - 'A';
        }

        p++;
    }
}

bool SizedString::split(char chSeparator, SizedString &left, SizedString &right) {
    const uint8_t *p = data;
    const uint8_t *end = data + len;

    while (p < end && *p != chSeparator) {
        p++;
    }

    if (p >= end) {
        return false;
    }

    left.data = data;
    left.len = (size_t)(p - data);

    right.data = p + 1;
    right.len = (size_t)(end - p - 1);
    return true;
}

bool SizedString::split(const char *separator, SizedString &left, SizedString &right) {
    size_t lenSep = strlen(separator);
    const char *p = (const char *)data;
    const char *end = p + len - lenSep + 1;

    while (p < end) {
        if (*p != *separator && strncmp(p, separator, lenSep) == 0) {
            // Found it.
            left.data = data;
            left.len = size_t((uint8_t *)p - data);

            right.data = left.data + lenSep;
            right.len = len - left.len - lenSep;
            return true;
        }
        p++;
    }

    return false;

}

bool strIsInList(SizedString &str, SizedString *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.equal(arr[i])) {
            return true;
        }
    }

    return false;
}

bool istrIsInList(SizedString &str, SizedString *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.iEqual(arr[i])) {
            return true;
        }
    }

    return false;
}


#ifdef UNIT_TEST
#include "Unittest.h"


TEST(SizedString, ngxStrItoa) {
    char buf[32];
    SizedString s;

    s = ngxStrItoa(0, buf);
    ASSERT_TRUE(s == "0");

    s = ngxStrItoa(-1, buf);
    ASSERT_TRUE(s == "-1");

    s = ngxStrItoa(65535, buf);
    ASSERT_TRUE(s == "65535");

    s = ngxStrItoa(4294967295, buf);
    ASSERT_TRUE(s == "4294967295");

    s = ngxStrItoa(9223372036854775807L, buf);
    ASSERT_TRUE(s == "9223372036854775807");
}

TEST(SizedString, ngxStrStr) {
    uint8_t *s = (uint8_t *)"abcdedfghijk";
    SizedString str = make_ngx_str(s, strlen((const char *)s));
    int p;

    p = ngxStrStr(str, MAKE_NGX_STR("jk"));
    ASSERT_TRUE(p == (int)str.len - 2);

    p = ngxStrStr(str, MAKE_NGX_STR("jkl"));
    ASSERT_TRUE(p == -1);

    p = ngxStrStr(str, MAKE_NGX_STR("abc"));
    ASSERT_TRUE(p == 0);

    // First 'd'
    p = ngxStrStr(str, MAKE_NGX_STR("d"));
    ASSERT_TRUE(p == 3);

    p = ngxStrStr(ngx_str_null, MAKE_NGX_STR("dd"));
    ASSERT_TRUE(p == -1);

    str = MAKE_NGX_STR("abcde");
    str.len -= 1;
    p = ngxStrStr(str, MAKE_NGX_STR("cde"));
    ASSERT_TRUE(p == -1);
}

TEST(SizedString, ngxStrIStr) {
    uint8_t *s = (uint8_t *)"abcdedfghijk";
    SizedString str = make_ngx_str(s, strlen((const char *)s));
    int p;

    p = ngxStrIStr(str, MAKE_NGX_STR("JK"));
    ASSERT_TRUE(p == (int)str.len - 2);

    p = ngxStrIStr(str, MAKE_NGX_STR("JKL"));
    ASSERT_TRUE(p == -1);

    p = ngxStrIStr(str, MAKE_NGX_STR("ABC"));
    ASSERT_TRUE(p == 0);

    // First 'd'
    p = ngxStrIStr(str, MAKE_NGX_STR("D"));
    ASSERT_TRUE(p == 3);

    p = ngxStrIStr(ngx_str_null, MAKE_NGX_STR("DD"));
    ASSERT_TRUE(p == -1);

    str = MAKE_NGX_STR("ABCD");
    str.len -= 1;
    p = ngxStrIStr(str, MAKE_NGX_STR("CDE"));
    ASSERT_TRUE(p == -1);
}

TEST(SizedString, ngxStrrchr) {
    uint8_t *s = (uint8_t *)"abcdedfghijk";
    SizedString str = make_ngx_str(s, strlen((const char *)s));
    uint8_t *p;

    p = ngxStrrchr(str, 'k');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'k');

    p = ngxStrrchr(str, 'm');
    ASSERT_TRUE(p == nullptr);

    p = ngxStrrchr(str, 'a');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'a');

    p = ngxStrrchr(str, 'd');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'd');
    ASSERT_TRUE(p[1] == 'f');

    p = ngxStrrchr(ngx_str_null, 'd');
    ASSERT_TRUE(p == nullptr);
}

TEST(SizedString, ngxStrlchr) {
    uint8_t *s = (uint8_t *)"abcdedfghijk";
    SizedString str = make_ngx_str(s, strlen((const char *)s));
    uint8_t *p;

    p = ngxStrlchr(str, 'k');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'k');

    p = ngxStrlchr(str, 'm');
    ASSERT_TRUE(p == nullptr);

    p = ngxStrlchr(str, 'a');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'a');

    p = ngxStrlchr(str, 'd');
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(p[0] == 'd');
    ASSERT_TRUE(p[1] == 'e');
}

TEST(SizedString, ngxStrCmp) {
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("abc"), make_ngx_str("abc")) == 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str(""), make_ngx_str("")) == 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("AB"), make_ngx_str("AB")) == 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("A"), make_ngx_str("A")) == 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("A"), make_ngx_str("B")) < 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("B"), make_ngx_str("A")) > 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("a"), make_ngx_str("B")) > 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("Ab"), make_ngx_str("A")) > 0);
    ASSERT_TRUE(ngxStrCmp(make_ngx_str("A"), make_ngx_str("Ab")) < 0);

    ASSERT_TRUE(ngxStrICmp(make_ngx_str("abc"), make_ngx_str("abc")) == 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str(""), make_ngx_str("")) == 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("Ab"), make_ngx_str("aB")) == 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("A"), make_ngx_str("A")) == 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("a"), make_ngx_str("B")) < 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("A"), make_ngx_str("b")) < 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("Ab"), make_ngx_str("a")) > 0);
    ASSERT_TRUE(ngxStrICmp(make_ngx_str("A"), make_ngx_str("ab")) < 0);
}

TEST(SizedString, ngxStrStartsWith) {
    ASSERT_TRUE(ngxStrStartsWith(make_ngx_str(""), make_ngx_str("")));
    ASSERT_TRUE(ngxStrStartsWith(make_ngx_str("abc"), make_ngx_str("abc")));
    ASSERT_FALSE(ngxStrStartsWith(make_ngx_str(""), make_ngx_str("abcdef")));
    ASSERT_TRUE(ngxStrStartsWith(make_ngx_str("abcd"), make_ngx_str("ab")));
    ASSERT_TRUE(ngxStrStartsWith(make_ngx_str(""), NGX_NULL_STRING));
}

TEST(SizedString, ngxStrIStartsWith) {
    ASSERT_TRUE(ngxStrIStartsWith(make_ngx_str(""), make_ngx_str("")));
    ASSERT_TRUE(ngxStrIStartsWith(make_ngx_str("abc"), make_ngx_str("ABc")));
    ASSERT_FALSE(ngxStrIStartsWith(make_ngx_str(""), make_ngx_str("abcdef")));
    ASSERT_TRUE(ngxStrIStartsWith(make_ngx_str("aBcd"), make_ngx_str("ab")));
    ASSERT_TRUE(ngxStrIStartsWith(make_ngx_str(""), NGX_NULL_STRING));
    ASSERT_TRUE(ngxStrIStartsWith(NGX_NULL_STRING, NGX_NULL_STRING));
}

TEST(SizedString, ngxStrEndsWith) {
    ASSERT_TRUE(ngxStrEndsWith(make_ngx_str(""), make_ngx_str("")));
    ASSERT_TRUE(ngxStrEndsWith(make_ngx_str("dabc"), make_ngx_str("abc")));
    ASSERT_FALSE(ngxStrEndsWith(make_ngx_str(""), make_ngx_str("abcdef")));
    ASSERT_TRUE(ngxStrEndsWith(make_ngx_str("abc"), make_ngx_str("")));
    ASSERT_TRUE(ngxStrEndsWith(make_ngx_str(""), NGX_NULL_STRING));
    ASSERT_TRUE(ngxStrEndsWith(NGX_NULL_STRING, NGX_NULL_STRING));
}

TEST(SizedString, ngxStrSplit) {
    {
        SizedString s1 = make_ngx_str("ab@c@d");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr);
        ASSERT_EQ(listStr.size(), 3);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab");

        ++it;
        ASSERT_EQ(*it, "c");

        ++it;
        ASSERT_EQ(*it, "d");
    }

    {
        SizedString s1 = make_ngx_str("ab@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr);
        ASSERT_EQ(listStr.size(), 2);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab");

        ++it;
        ASSERT_EQ(*it, "");
    }

    {
        SizedString s1 = make_ngx_str("ab@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr, 0);
        ASSERT_EQ(listStr.size(), 1);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab@");
    }

    {
        SizedString s1 = make_ngx_str("ab@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr, 1);
        ASSERT_EQ(listStr.size(), 1);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab@");
    }

    {
        SizedString s1 = make_ngx_str("ab@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr, 2);
        ASSERT_EQ(listStr.size(), 2);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab");

        ++it;
        ASSERT_EQ(*it, "");
    }

    {
        SizedString s1 = make_ngx_str("ab@@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr, 3);
        ASSERT_EQ(listStr.size(), 3);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab");

        ++it;
        ASSERT_EQ(*it, "");

        ++it;
        ASSERT_EQ(*it, "");
    }

    {
        SizedString s1 = make_ngx_str("ab@c@");
        list_ngx_str_t listStr;
        ngxStrSplit(s1, '@', listStr, 3);
        ASSERT_EQ(listStr.size(), 3);
        list_ngx_str_t::iterator it = listStr.begin();
        ASSERT_EQ(*it, "ab");

        ++it;
        ASSERT_EQ(*it, "c");

        ++it;
        ASSERT_EQ(*it, "");
    }
}

TEST(SizedString, strip) {
    {
        SizedString s1 = make_ngx_str("ab");
        ngxStrTrim(s1, make_ngx_str(" \t"));
        ASSERT_TRUE(s1 == "ab");
    }

    {
        SizedString s1 = make_ngx_str(" ab ");
        ngxStrTrim(s1, make_ngx_str(" \t"));
        ASSERT_TRUE(s1 == "ab");
    }

    {
        SizedString s1 = make_ngx_str(" \ta b\t ");
        ngxStrTrim(s1, make_ngx_str(" \t"));
        ASSERT_TRUE(s1 == "a b");
    }
}

TEST(SizedString, NullString) {
    {
        SizedString another = MAKE_NGX_STR("abc");
        SizedString in = { 0, nullptr };

        ngxStrTrim(in, 'a');
        ngxStrTrim(in, another);
        ngxStrCmp(in, another);
        ngxStrICmp(in, another);

        list_ngx_str_t strs;
        ngxStrSplit(in, 'a', strs);
        ngxStrStartsWith(in, another);
        ngxStrIStartsWith(in, another);
    }
}

#endif
