// Semaphore.h: interface for the Semaphore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_)
#define AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SEM_LOCKED   0
#define SEM_UNLOCKED 1

#define WAIT_FOREVER INFINITE

class Semaphore  
{
public:
    Semaphore(int nCount = SEM_LOCKED);
    virtual ~Semaphore();

    bool wait(int ms = WAIT_FOREVER); // returns false if it times out
    void signal();
 
 private:
    int            m_nCount;
       HANDLE        m_hSemaphore;

};

#endif // !defined(AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_)
