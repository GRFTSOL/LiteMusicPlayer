#include "SJVM.h"
#include "SjvmLib.h"

//////////////////////////////////////////////////////////////////////////

class CSjobjUnitTest : public CJObject
{
    DECLARE_JNI_OBJ(CSjobjUnitTest)
public:
    CSjobjUnitTest() : CJObject(m_sTypeId, 0) { }

    static void registerJNI()
    {
        int        nRet;

        // add class members definition.
        CClass *pClass = g_sjvm->getClass(CLASS_NAME);
        assert(pClass);

        nRet = pClass->addNativeMethod(true, "failedMessage", (void *)failedMessage, TID_VOID, TID_STRING, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(true, "message", (void *)messageStr, TID_VOID, TID_STRING, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(true, "message", (void *)messageInt, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(true, "getSp", (void *)getSp, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(true, "getBp", (void *)getBp, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(true, "printJVMStatus", (void *)printJVMStatus, TID_VOID, TID_INVALID);
        assert(nRet != -1);
    }

    static void SJNI_API messageStr(cstr_t szMsg)
    {
        printf("%s\n", szMsg);
    }

    static void SJNI_API messageInt(int value)
    {
        printf("%d\n", value);
    }

    static void SJNI_API failedMessage(cstr_t szMsg)
    {
#ifdef _WIN32
        messageBox(nullptr, szMsg, "SimpleJVM UnitTest", MB_OK | MB_TOPMOST);
        assert(0);
#else
        printf("%s\n", szMsg);
#endif
    }

    static int SJNI_API getSp()
    {
        return (int)g_sjvm->m_stack.size();
    }

    static int SJNI_API getBp()
    {
        return (int)g_sjvm->m_bp;
    }

    static void SJNI_API printJVMStatus()
    {
        printf(">>>SimpleJVM internal status:\n");
        printf("BP: %d, SP: %d,    Static variable count: %d,    \n", (int)g_sjvm->m_bp, (int)g_sjvm->m_stack.size(), (int)g_sjvm->m_vStaticVars.size());
        printf("JObject manager size: %d,    free: %d\n", (int)g_sjvm->m_jobjMgr.m_vJObjects.size(), (int)g_sjvm->m_jobjMgr.m_vFreeJObjs.size());
        printf("<<<End of status:\n");
    }

public:

};

IMPLENENT_JNI_OBJ(CSjobjUnitTest, "UnitTest")


IMPLENENT_JNI_OBJ(CSjobjException, "Exception")

void CSjobjException::registerJNI()
{
    int        nRet;

    // add class members definition.
    CClass *pClass = g_sjvm->getClass(CLASS_NAME);
    assert(pClass);

    nRet = pClass->addNativeMethod(false, "Exception", (void *)ExceptionConstructor, TID_VOID, TID_STRING, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(false, "toString", (void *)toString, TID_STRING, TID_INVALID);
    assert(nRet != -1);
}

void SJNI_API CSjobjException::ExceptionConstructor(jobject jObj, cstr_t szException)
{
    CSjobjException    *pException = (CSjobjException *)g_sjvm->getJObject(jObj);
    assert(pException->m_typeId == m_sTypeId || g_sjvm->getClassTable().isSubClassOf(pException->m_typeId, m_sTypeId));
    pException->strException = szException;
}

jobject SJNI_API CSjobjException::toString(jobject jObj)
{
    CSjobjException    *pException = (CSjobjException *)g_sjvm->getJObject(jObj);
    assert(pException->m_typeId == m_sTypeId || g_sjvm->getClassTable().isSubClassOf(pException->m_typeId, m_sTypeId));
    return g_sjvm->newString(pException->strException.c_str(), true);
}

void initSjvmLib(CSJVM *sjvm)
{
    CSjobjUnitTest::registerClassType(sjvm);
    CSjobjException::registerClassType(sjvm);

    CSjobjUnitTest::registerJNI();
    CSjobjException::registerJNI();
}
