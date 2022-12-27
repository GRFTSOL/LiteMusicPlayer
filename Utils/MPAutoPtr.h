#pragma once

//
//  MPAutoPtr.h
//  Mp3Player
//
//  Created by HongyongXiao on 2021/12/22.
//

#ifndef MPAutoPtr_h
#define MPAutoPtr_h

#include <atomic>


#define OBJ_REFERENCE_DECL    \
public:\
    virtual void addRef() {\
        m_nReference++;\
    }\
    virtual void release() {\
        if (--m_nReference == 0)\
            delete this;\
    }\
protected:\
    std::atomic<long>            m_nReference;

#define OBJ_REFERENCE_INIT    \
    m_nReference = 0;

#define NEW_OBJ_REFERENCE(_class)\
    { _class *p = new _class(); p->addRef() }\

template <class T>
class CMPAutoPtr {
public:
    typedef T _PtrClass;
    CMPAutoPtr() { p = nullptr; }
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

    operator T* () const { return (T*)p; }
    T& operator * () const { assert(p != nullptr); return *p; }

    //The assert on operator& usually indicates a bug.  If this is really
    //what is needed, however, take the address of the p member explicitly.
    T** operator & () { assert(p == nullptr); return &p; }
    T* operator -> () const { assert(p != nullptr); return p; }
    T* operator = (T* lp) {
        if (lp) {
            lp->addRef();
        }

        if (p) {
            (p)->release();
        }
        p = lp;
        return (T*)lp;
    }

    T* operator = (const CMPAutoPtr<T>& lp) {
        if (lp.p) {
            lp.p->addRef();
        }
        if (p) {
            (p)->release();
        }
        p = lp.p;
        return (T*)lp.p;
    }

    bool operator ! () const { return (p == nullptr); }
    bool operator < (T* pT) const { return p < pT; }
    bool operator == (T* pT) const { return p == pT; }

    T                           *p;

};

#endif /* MPAutoPtr_h */
