#pragma once

#include "IMPlayer.h"

void calc_freq(unsigned char *dest, unsigned char *src);

class CXStr : public IXStr
{
OBJ_REFERENCE_DECL
public:
    CXStr();
    virtual ~CXStr();

    virtual LPXSTR data();
    virtual LPCXSTR c_str();
    virtual size_t size();
    virtual uint32_t capacity();
    virtual void resize(uint32_t nSize);
    virtual MLRESULT reserve(uint32_t nCapacity);
    virtual void copy(IXStr *pSrcStr);
    virtual void copy(LPCXSTR str);
    virtual void erase(int nOffset, int n);
    virtual void clear();
    virtual void insert(int nOffset, LPCXSTR str, int n);
    virtual void append(LPCXSTR str, int n);

protected:
    string        m_str;

};

class CVXStr : public IVXStr
{
OBJ_REFERENCE_DECL
public:
    CVXStr();
    virtual ~CVXStr();

    virtual size_t size();
    virtual void push_back(LPCXSTR szStr);
    virtual void clear();
    virtual LPCXSTR at(int index);
    virtual void set(int index, LPCXSTR szStr);
    virtual void insert(int index, cstr_t szStr);

protected:
    vector<string>        m_vstr;

};

class CVInt : public IVInt
{
    OBJ_REFERENCE_DECL
public:
    CVInt();
    virtual ~CVInt();

    virtual size_t size();
    virtual void push_back(int nData);
    virtual void clear();
    virtual int at(int index);
    virtual void set(int index, int nData);
    virtual void insert(int index, int nData);

protected:
    vector<int>            m_vInt;

};

class CVector : public IVector
{
    OBJ_REFERENCE_DECL
public:
    CVector() { OBJ_REFERENCE_INIT }
    virtual ~CVector() { }

    virtual size_t size() { return m_vData.size(); }
    virtual void push_back(void *p) { m_vData.push_back(p); }
    virtual void clear() { m_vData.clear(); }
    virtual void *at(int index)
        { if (index >= 0 && index < (int)m_vData.size()) return m_vData[index]; else return nullptr; }
    virtual void set(int index, void *p)
        { if (index >= 0 && index < (int)m_vData.size()) m_vData[index] = p; }
    virtual void insert(int index, void *p)
        { if (index >= 0 && index < (int)m_vData.size()) m_vData.insert(m_vData.begin() + index, p); }

protected:
    vector<void *>            m_vData;

};
