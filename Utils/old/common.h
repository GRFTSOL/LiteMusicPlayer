#pragma once

#ifndef _ML_COMMON_INC_
#define _ML_COMMON_INC_

enum FILE_ENCODING {
    FE_ANSI,
    FE_UCS2,
    FE_UCS2_BE,
    FE_UTF8
};

#define SZ_FE_UCS2          "\xFF\xFE"
#define SZ_FE_UCS2_BE       "\xFE\xFF"
#define SZ_FE_UTF8          "\xEF\xBB\xBF"

template<int n> struct __sizeof_at_compile_helper;
#define sizeof_at_compile(obj)  __sizeof_at_compile_helper<sizeof(obj)> v

#define CountOf(arr)        (sizeof(arr) / sizeof(arr[0]))

#define isEmptyString(str)  ((str) == nullptr || (str)[0] == '\0')

#define emptyStr(str)       ((str)[0] = '\0')

#define IsFlagValid(value, flag) (((value) & (flag)) == (flg))
#define isFlagSet(value, flag)  (((value) & (flag)) == (flag))

#define tobool(bValue)      ((bValue) != 0)

inline uint32_t uint32FromBE(uint8_t *byData) {
    return byData[3] | (byData[2] << 8) | (byData[1] << 16) | (byData[0] << 24);
}

inline void uint32ToBE(uint32_t value, uint8_t *byData) {
    byData[3] = value & 0xFF;
    byData[2] = (value >> 8) & 0xFF;
    byData[1] = (value >> 16) & 0xFF;
    byData[0] = (value >> 24) & 0xFF;
}

inline uint32_t uint16FromBE(uint8_t *byData) {
    return byData[1] | (byData[0] << 8);
}

inline void uint16ToBE(uint32_t value, uint8_t *byData) {
    byData[1] = value & 0xFF;
    byData[0] = (value >> 8) & 0xFF;
}

inline uint32_t uint32FromLE(uint8_t *byData) {
    return byData[0] | (byData[1] << 8) | (byData[2] << 16) | (byData[3] << 24);
}

inline void uint32ToLE(uint32_t value, uint8_t *byData) {
    byData[0] = value & 0xFF;
    byData[1] = (value >> 8) & 0xFF;
    byData[2] = (value >> 16) & 0xFF;
    byData[3] = (value >> 24) & 0xFF;
}

inline uint16_t uint16FromLE(uint8_t *byData) {
    return byData[0] | (byData[1] << 8);
}

inline void uint16ToLE(uint16_t value, uint8_t *byData) {
    byData[0] = value & 0xFF;
    byData[1] = (value >> 8) & 0xFF;
}

template<class int_t1, class int_t2>
int_t1 inline max_fun(int_t1 a, int_t2 b) {
    return (((a) > (b)) ? (a) : (b));
}

template<class int_t1, class int_t2>
int_t1 inline min_fun(int_t1 a, int_t2 b) {
    return (((a) < (b)) ? (a) : (b));
}

#ifdef _WIN32
#define PATH_SEP_CHAR       '\\'
#define PATH_SEP_STR        "\\"
#else
#define PATH_SEP_CHAR       '/'
#define PATH_SEP_STR        "/"
#endif

#define CHAR_SPACE        ' '
#define CHAR_TAB            '\t'
#define CHAR_NEWLINE        '\n'
#define CHAR_RETURN         '\r'
#define CHAR_NULL           '\0'
#define CHAR_COMMA          ','
#define CHAR_SEMICOLON      ';'

#define SZ_RETURN           "\r\n"

#ifndef _WIN32
#define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)((a) & 0xff)) | ((uint16_t)((uint8_t)((b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((uint16_t)((a) & 0xffff)) | ((uint32_t)((uint16_t)((b) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16_t)((l) & 0xffff))
#define HIWORD(l)           ((uint16_t)((l) >> 16))
#define LOBYTE(w)           ((uint8_t)((w) & 0xff))
#define HIBYTE(w)           ((uint8_t)((w) >> 8))
#endif

#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))

#define GetRValue(rgb)      ((uint8_t)(rgb))
#define GetGValue(rgb)      ((uint8_t)(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)      ((uint8_t)((rgb)>>16))
#endif

#ifndef _WIN32

inline void interlockedIncrement(long *nReference) {
    (*nReference)++;
}

inline long interlockedDecrement(long *nReference) {
    return --(*nReference);
}

#endif

#define OBJ_REFERENCE_DECL    \
public:\
    virtual void addRef() {\
        interlockedIncrement(&m_nReference);\
    }\
    virtual void release() {\
        if (interlockedDecrement(&m_nReference) == 0)\
            delete this;\
    }\
protected:\
    long            m_nReference;

#define OBJ_REFERENCE_INIT    \
    m_nReference = 0;

#define NEW_OBJ_REFERENCE(_class)\
    { _class *p = new _class(); p->addRef() }\

template <class T>
class CMPAutoPtr {
public:
    typedef T _PtrClass;
    CMPAutoPtr() {
        p=nullptr;
    }
    CMPAutoPtr(T* lp) {
        if ((p = lp) != nullptr) {
            p->addRef();
        }
    }
    CMPAutoPtr(const CMPAutoPtr<T>& lp) {
        if ((p = lp.p) != nullptr) {
            p->addRef();
        }
    }
    ~CMPAutoPtr() {
        if (p) {
            p->release();
        }
    }
    void release() {
        if (p) {
            p->release();
            p = nullptr;
        }
    }
    operator T*() const {
        return (T*)p;
    }
    T& operator*() const {
        assert(p!=nullptr);
        return *p;
    }
    //The assert on operator& usually indicates a bug.  If this is really
    //what is needed, however, take the address of the p member explicitly.
    T** operator&() {
        assert(p==nullptr);
        return &p;
    }
    T* operator->() const {
        assert(p!=nullptr);
        return p;
    }
    T* operator=(T* lp) {
        if (lp) {
            lp->addRef();
        }
        if (p) {
            (p)->release();
        }
        p = lp;
        return (T*)lp;
    }
    T* operator=(const CMPAutoPtr<T>& lp) {
        if (lp.p) {
            lp.p->addRef();
        }
        if (p) {
            (p)->release();
        }
        p = lp.p;
        return (T*)lp.p;
    }
    bool operator!() const {
        return (p == nullptr);
    }
    bool operator<(T* pT) const {
        return p < pT;
    }
    bool operator==(T* pT) const {
        return p == pT;
    }

    T* p;
};

#ifdef _WIN32

// Simplified bstr
class bstr_s {
public:
    bstr_s() { m_str = nullptr; }
    bstr_s(const OLECHAR * psz) { m_str = SysAllocString(psz); }
    virtual ~bstr_s() { free(); }

    void free() { if (m_str) { SysFreeString(m_str); m_str = nullptr; } }
    BSTR c_str() const { return m_str; }
    operator BSTR() const { return m_str; }

    BSTR* operator&() {
        assert(m_str == nullptr);
        return &m_str;
    }

protected:
    BSTR                        m_str;

};

#endif

#endif // _ML_COMMON_INC_
