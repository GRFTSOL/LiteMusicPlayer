#include "MPTools.h"
#include "fft.h"
#include <math.h>


void calc_freq(unsigned char *dest, unsigned char *src) {
    static fft_state *state = nullptr;
    float tmp_out[257];
    int i;

    if(!state) {
        state = fft_init();
    }

    fft_perform((char*)src,tmp_out,state);

    for(i = 0; i < 256; i++) {
        dest[i] = (unsigned char)(((int)sqrt(tmp_out[i + 1])) >> 4);
    }
}

//////////////////////////////////////////////////////////////////////////
// class CXStr

CXStr::CXStr() {
    OBJ_REFERENCE_INIT
}

CXStr::~CXStr() {

}

char * CXStr::data() {
    return (char *)m_str.data();
}

cstr_t CXStr::c_str() {
    return m_str.c_str();
}

size_t CXStr::size() {
    return m_str.size();
}

uint32_t CXStr::capacity() {
    return (uint32_t)m_str.capacity();
}

void CXStr::resize(uint32_t nSize) {
    m_str.resize(nSize);
}

MLRESULT CXStr::reserve(uint32_t nCapacity) {
    m_str.resize(nCapacity);
    return ERR_OK;
}

void CXStr::copy(IString *pSrcStr) {
    m_str.clear();
    m_str.append(pSrcStr->c_str(), pSrcStr->size());
}

void CXStr::copy(cstr_t str) {
    m_str = str;
}

void CXStr::erase(int nOffset, int n) {
    m_str.erase(nOffset, n);
}

void CXStr::clear() {
    m_str.clear();
}

void CXStr::insert(int nOffset, cstr_t str, int n) {
    m_str.insert(nOffset, str, n);
}

void CXStr::append(cstr_t str, int n) {
    m_str.append(str, n);
}

//////////////////////////////////////////////////////////////////////////

CVXStr::CVXStr() {
    OBJ_REFERENCE_INIT
}

CVXStr::~CVXStr() {

}

size_t CVXStr::size() {
    return m_vstr.size();
}

void CVXStr::push_back(cstr_t szStr) {
    m_vstr.push_back(szStr);
}

void CVXStr::clear() {
    m_vstr.clear();
}

cstr_t CVXStr::at(int index) {
    assert(index >= 0 && index < (int)m_vstr.size());
    if (index >= 0 && index < (int)m_vstr.size()) {
        return m_vstr[index].c_str();
    } else {
        return "";
    }
}

void CVXStr::set(int index, cstr_t szStr) {
    assert(index >= 0 && index < (int)m_vstr.size());
    if (index >= 0 && index < (int)m_vstr.size()) {
        m_vstr[index] = szStr;
    }
}

void CVXStr::insert(int index, cstr_t szStr) {
    assert(index >= 0 && index < (int)m_vstr.size());
    if (index >= 0 && index < (int)m_vstr.size()) {
        m_vstr.insert(m_vstr.begin() + index, szStr);
    } else {
        m_vstr.push_back(szStr);
    }
}


CVInt::CVInt() {

}

CVInt::~CVInt() {

}

size_t CVInt::size() {
    return m_vInt.size();
}

void CVInt::push_back(int nData) {
    m_vInt.push_back(nData);
}

void CVInt::clear() {
    m_vInt.clear();
}

int CVInt::at(int index) {
    assert(index >= 0 && index < (int)m_vInt.size());
    if (index >= 0 && index < (int)m_vInt.size()) {
        return m_vInt[index];
    } else {
        return 0;
    }
}

void CVInt::set(int index, int nData) {
    assert(index >= 0 && index < (int)m_vInt.size());
    if (index >= 0 && index < (int)m_vInt.size()) {
        m_vInt[index] = nData;
    }
}

void CVInt::insert(int index, int nData) {
    assert(index >= 0 && index < (int)m_vInt.size());
    if (index >= 0 && index < (int)m_vInt.size()) {
        m_vInt.insert(m_vInt.begin() + index, nData);
    } else {
        m_vInt.push_back(nData);
    }
}
