#include "BatchMem.h"

#define BLOCK_HEAD_SIZE        sizeof(int)
#define ALIGN_SIZE            sizeof(int)

CBatchMem::CBatchMem(int nBlockSize)
{
    m_nBlockSize = nBlockSize - BLOCK_HEAD_SIZE;
    m_pCurBlock = nullptr;
}

CBatchMem::~CBatchMem(void)
{
    freeAll();
}

void *CBatchMem::alloc(size_t nSize)
{
    if (m_pCurBlock == nullptr)
    {
        m_pCurBlock = (Block *)new char[m_nBlockSize + BLOCK_HEAD_SIZE];
        m_pCurBlock->nUsedSize = BLOCK_HEAD_SIZE;
    }

    if (m_pCurBlock->nUsedSize + nSize >= m_nBlockSize)
    {
        if (nSize > m_nBlockSize)
        {
            char    *p = new char[nSize];
            m_listBlocks.push_back(p);
            return p;
        }
        m_listBlocks.push_back((char *)m_pCurBlock);
        m_pCurBlock = (Block*)(new char[m_nBlockSize + BLOCK_HEAD_SIZE]);
        m_pCurBlock->nUsedSize = BLOCK_HEAD_SIZE;
    }

    char    *p = m_pCurBlock->mem + m_pCurBlock->nUsedSize;
    m_pCurBlock->nUsedSize = (m_pCurBlock->nUsedSize + nSize + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);

    return p;
}

char *CBatchMem::dupStr(cstr_t szString)
{
    size_t    len = strlen(szString) + 1;
    char    *p = (char *)alloc(len);
    assert(p);
    memcpy(p, szString, len);
    return p;
}

void CBatchMem::freeAll()
{
    if (m_pCurBlock)
    {
        delete[] (char *)m_pCurBlock;
        m_pCurBlock = nullptr;
    }

    ListBlocks::iterator it;
    for (it = m_listBlocks.begin(); it != m_listBlocks.end(); ++it)
    {
        delete [] (*it);
    }
    m_listBlocks.clear();
}
