// MLHandleMap.h: interface for the CMLHandleMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ML_HANDLE_MAP_INC_
#define _ML_HANDLE_MAP_INC_

#include <mutex>
#include <map>


template <class handle_t, class object_t>
class CMLHandleMap
{
public:
    CMLHandleMap() {
    }

    ~CMLHandleMap()
    {
        MutexAutolock autoLock(m_mutex);

        for (auto it = m_map.begin(); it != m_map.end(); it++)
        {
            (*it).second->detach();
            delete (*it).second;
        }

        m_map.clear();
    }

    object_t *fromhandle(handle_t handle)
    {
        MutexAutolock autoLock(m_mutex);

        auto it = m_map.find(handle);
        if (it == m_map.end())
        {
            object_t    *pObj;
            pObj = new object_t();
            m_map[handle] = pObj;
            pObj->attach(handle);
            return pObj;
        }
        else
        {
            return (*it).second;
        }
    }

    void remove(handle_t handle)
    {
        MutexAutolock autoLock(m_mutex);

        auto it = m_map.find(handle);
        if (it != m_map.end())
        {
            (*it).second->detach();
            delete (*it).second;
            m_map.erase(it);
        }
    }

protected:
    std::map<handle_t, object_t *>  m_map;
    std::mutex                      m_mutex;

};

#endif // _ML_HANDLE_MAP_INC_
