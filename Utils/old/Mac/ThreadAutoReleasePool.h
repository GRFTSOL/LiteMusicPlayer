#pragma once

#include "Thread.h"

class CThreadAutoReleasePool : public CThread
{
public:
    bool create(FUNThread function, void* lpData);

protected:
    static void *threadFunction(void *lpParam);

};
