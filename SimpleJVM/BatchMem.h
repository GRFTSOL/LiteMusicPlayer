#pragma once

#include "../Utils/Utils.h"


class CBatchMem
{
public:
    CBatchMem(int nBlockSize = 1024 * 8);
    virtual ~CBatchMem(void);

    void *alloc(size_t nSize);
    char *dupStr(cstr_t szString);

    void freeAll();

protected:
    struct Block {
        size_t        nUsedSize;
        char        mem[sizeof(int)];
    };
    typedef std::list<char *>   ListBlocks;

    size_t                      m_nBlockSize;

    ListBlocks                  m_listBlocks;

    Block                       *m_pCurBlock;

};
