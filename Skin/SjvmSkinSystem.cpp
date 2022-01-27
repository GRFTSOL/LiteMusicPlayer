#include "SkinTypes.h"
#include "Skin.h"

#include "SjvmSkinSystem.h"
#include "../SimpleJVM/SjvmLib.h"

#define TID_SKIN_WND_NOTIFY_EVENT    CSjvmSkinWndEventObj::m_sTypeId
#define TID_SKIN_WND                CSjvmSkinWnd::m_sTypeId
#define TID_UIOBJECT                CSjvmUIObject::m_sTypeId

//////////////////////////////////////////////////////////////////////////


class CSjvmSkinWnd : public CJObject
{
    DECLARE_JNI_OBJ(CSjvmSkinWnd)
public:
    CSjvmSkinWnd() : CJObject(m_sTypeId, 0)
    {
        m_pSkinWnd = nullptr;
    }
    virtual ~CSjvmSkinWnd()
    {
        for (SetInt::iterator it = m_setTimers.begin(); it != m_setTimers.end(); ++it)
        {
            m_pSkinWnd->killTimer(*it);
        }
        m_setTimers.clear();
    }

    static void registerJNI()
    {
        int        nRet;

        // add class members definition.
        CClass *pClass = g_sjvm->getClass(CLASS_NAME);
        assert(pClass);

        pClass->addNativeDefaultConstructor();

        nRet = pClass->addNativeMethod(false, "getUIObject", (void *)getUIObject, TID_UIOBJECT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "setUIObjVisible", (void *)setUIObjVisible, TID_VOID, TID_INT, TID_BOOLEAN, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "getUIObjVisible", (void *)getUIObjVisible, TID_BOOLEAN, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "setUIObjProperty", (void *)setUIObjProperty, TID_VOID, TID_INT, TID_STRING, TID_STRING, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "setProperty", (void *)setProperty, TID_VOID, TID_STRING, TID_STRING, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "exeCommand", (void *)exeCommand, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "getWndPositionX", (void *)getWndPositionX, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "getWndPositionY", (void *)getWndPositionY, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "getWndWidth", (void *)getWndWidth, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "getWndHeight", (void *)getWndHeight, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "setWndPosition", (void *)setWndPosition, TID_VOID, TID_INT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "moveWindow", (void *)moveWindow, TID_VOID, TID_INT, TID_INT, TID_INT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "registerTimer", (void *)registerTimer, TID_INT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "unregisterTimer", (void *)unregisterTimer, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "startAnimation", (void *)startAnimation, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);

        nRet = pClass->addNativeMethod(false, "stopAnimation", (void *)stopAnimation, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);
    }

public:
    static jobject SJNI_API getUIObject(jobject thiz, int nId)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return JOBJ_NULL;

        CUIObject    *pObj = pSkinWnd->getUIObjectById(nId);
        if (!pObj)
            return JOBJ_NULL;

        jobject        jobj;
        CSjvmUIObject *pJObj = (CSjvmUIObject*)g_sjvm->newJObject(TID_UIOBJECT, jobj);
        assert(pJObj && pJObj->m_typeId == TID_UIOBJECT);

        pJObj->m_pUIObject = pObj;
        pJObj->m_pSkinWnd = pSkinWnd;

        return jobj;
    }

    static void SJNI_API setUIObjVisible(jobject thiz, int nId, bool bVisible)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        CUIObject *pObj = pSkinWnd->getUIObjectById(nId);
        if (pObj)
            pObj->setVisible(bVisible);
    }

    static bool SJNI_API getUIObjVisible(jobject thiz, int nId)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return false;

        CUIObject *pObj = pSkinWnd->getUIObjectById(nId);
        if (pObj)
            return pObj->isVisible();

        return false;
    }

    static void SJNI_API setUIObjProperty(jobject thiz, int nId, cstr_t name, cstr_t value)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        CUIObject *pObj = pSkinWnd->getUIObjectById(nId);
        if (pObj)
            pObj->setProperty(name, value);
    }

    static void SJNI_API setProperty(jobject thiz, cstr_t name, cstr_t value)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        pSkinWnd->setProperty(name, value);
    }

    static void SJNI_API exeCommand(jobject thiz, int nCmdId)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        pSkinWnd->onCustomCommand(nCmdId);
    }

    static int SJNI_API getWndPositionX(jobject thiz)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return 0;

        return pSkinWnd->m_rcBoundBox.left;
    }

    static int SJNI_API getWndPositionY(jobject thiz)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return 0;

        return pSkinWnd->m_rcBoundBox.top;
    }

    static int SJNI_API getWndWidth(jobject thiz)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return 0;

        return pSkinWnd->m_rcBoundBox.width();
    }

    static int SJNI_API getWndHeight(jobject thiz)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return 0;

        return pSkinWnd->m_rcBoundBox.height();
    }

    static void SJNI_API setWndPosition(jobject thiz, int x, int y)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        pSkinWnd->setWindowPos(x, y);
    }

    static void SJNI_API moveWindow(jobject thiz, int x, int y, int width, int height)
    {
        CSkinWnd    *pSkinWnd = getSkinWndOfThis(thiz);
        assert(pSkinWnd);
        if (!pSkinWnd)
            return;

        pSkinWnd->moveWindow(x, y, width, height, true);
    }

    static int SJNI_API registerTimer(jobject thiz, int nTimeOut)
    {
        CSjvmSkinWnd    *pThis = getThis(thiz);
        assert(pThis);
        if (!pThis)
            return 0;

        CSkinWnd        *pSkinWnd = pThis->m_pSkinWnd;

        int        nTimerId = pSkinWnd->m_timerIDMax;
        pSkinWnd->m_timerIDMax++;
        pThis->m_setTimers.insert(nTimerId);

        pSkinWnd->setTimer(nTimerId, nTimeOut);

        return nTimerId;
    }

    static void SJNI_API unregisterTimer(jobject thiz, int nTimerId)
    {
        CSjvmSkinWnd    *pThis = getThis(thiz);
        assert(pThis);
        if (!pThis)
            return;

        CSkinWnd        *pSkinWnd = pThis->m_pSkinWnd;

        pThis->m_setTimers.erase(nTimerId);

        pSkinWnd->killTimer(nTimerId);
    }

    static void SJNI_API startAnimation(jobject thiz, int nAnimationId)
    {
        CSjvmSkinWnd    *pThis = getThis(thiz);
        assert(pThis);
        if (!pThis)
            return;

        CSkinWnd        *pSkinWnd = pThis->m_pSkinWnd;

        pSkinWnd->startAnimation(nAnimationId);
    }

    static void SJNI_API stopAnimation(jobject thiz, int nAnimationId)
    {
        CSjvmSkinWnd    *pThis = getThis(thiz);
        assert(pThis);
        if (!pThis)
            return;

        CSkinWnd        *pSkinWnd = pThis->m_pSkinWnd;

        pSkinWnd->stopAnimation(nAnimationId);
    }

protected:
    static CSkinWnd *getSkinWndOfThis(jobject thiz)
    {
        CSjvmSkinWnd    *pThis = (CSjvmSkinWnd*)g_sjvm->getJObject(thiz);
        assert(pThis && pThis->m_sTypeId == TID_SKIN_WND);
        if (!pThis)
            return nullptr;

        assert(pThis->m_pSkinWnd && CSjvmSkinSystem::m_pSkinFactory->isValidSkinWnd(pThis->m_pSkinWnd));
        if (!CSjvmSkinSystem::m_pSkinFactory->isValidSkinWnd(pThis->m_pSkinWnd))
            return nullptr;

        return pThis->m_pSkinWnd;
    }

    static CSjvmSkinWnd *getThis(jobject thiz)
    {
        CSjvmSkinWnd    *pThis = (CSjvmSkinWnd*)g_sjvm->getJObject(thiz);
        assert(pThis && pThis->m_sTypeId == TID_SKIN_WND);
        if (!pThis)
            return nullptr;

        assert(pThis->m_pSkinWnd && CSjvmSkinSystem::m_pSkinFactory->isValidSkinWnd(pThis->m_pSkinWnd));
        if (!CSjvmSkinSystem::m_pSkinFactory->isValidSkinWnd(pThis->m_pSkinWnd))
            return nullptr;

        return pThis;
    }

public:
    CSkinWnd                *m_pSkinWnd;
    SetInt                    m_setTimers;

};

IMPLENENT_JNI_OBJ(CSjvmSkinWnd, "SkinWnd")

//////////////////////////////////////////////////////////////////////////

class CSjvmSkinWndEventObj : public CJObject
{
    DECLARE_JNI_OBJ(CSjvmSkinWndEventObj)
public:
    CSjvmSkinWndEventObj() : CJObject(m_sTypeId, 0) { }

    static bool SJNI_API onCommand(jobject thiz, int nCmdId) { return false; }
    static void SJNI_API onCreate(jobject thiz) { }
    static void SJNI_API onDestroy(jobject thiz) { }
    static void SJNI_API onSize(jobject thiz, int cx, int cy) { }
    static void SJNI_API onMove(jobject thiz, int x, int y) { }
    static void SJNI_API onActivate(jobject thiz, bool bActived) { }
    static void SJNI_API onMouseActive(jobject thiz, bool bMouseActive) { }
    static void SJNI_API onTimer(jobject thiz, int nTimerId) { }

    static void registerJNI()
    {
        int        nRet;

        // add class members definition.
        CClass *pClass = g_sjvm->getClass(CLASS_NAME);
        assert(pClass);

        pClass->addNativeDefaultConstructor();

        SjvmSkinWndEvent::m_sOnCommand = nRet = pClass->addNativeMethod(false, "onCommand", (void *)onCommand, TID_BOOLEAN, TID_INT, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnCreate = nRet = pClass->addNativeMethod(false, "onCreate", (void *)onCreate, TID_VOID, TID_SKIN_WND, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnDestroy = nRet = pClass->addNativeMethod(false, "onDestroy", (void *)onDestroy, TID_VOID, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnSize = nRet = pClass->addNativeMethod(false, "onSize", (void *)onSize, TID_VOID, TID_INT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnMove = nRet = pClass->addNativeMethod(false, "onMove", (void *)onMove, TID_VOID, TID_INT, TID_INT, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnActivate = nRet = pClass->addNativeMethod(false, "onActivate", (void *)onActivate, TID_VOID, TID_BOOLEAN, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnMouseActive = nRet = pClass->addNativeMethod(false, "onMouseActive", (void *)onMouseActive, TID_VOID, TID_BOOLEAN, TID_INVALID);
        assert(nRet != -1);

        SjvmSkinWndEvent::m_sOnTimer = nRet = pClass->addNativeMethod(false, "onTimer", (void *)onTimer, TID_VOID, TID_INT, TID_INVALID);
        assert(nRet != -1);
    }

};

IMPLENENT_JNI_OBJ(CSjvmSkinWndEventObj, "SkinWndEvent")

//////////////////////////////////////////////////////////////////////////

int        SjvmSkinWndEvent::m_sOnCommand = 0;
int        SjvmSkinWndEvent::m_sOnCreate = 0;
int        SjvmSkinWndEvent::m_sOnDestroy = 0;
int        SjvmSkinWndEvent::m_sOnSize = 0;
int        SjvmSkinWndEvent::m_sOnMove = 0;
int        SjvmSkinWndEvent::m_sOnActivate = 0;
int        SjvmSkinWndEvent::m_sOnMouseActive = 0;
int        SjvmSkinWndEvent::m_sOnTimer = 0;

// Return true, the command will not be processed down.
bool SjvmSkinWndEvent::onCommand(int nCmdId)
{
    if (m_sOnCommand >= 0)
    {
        return g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnCommand, TID_INT, nCmdId);
    }

    return false;
}

void SjvmSkinWndEvent::onCreate(CSkinWnd *pWnd)
{
    if (m_sOnCreate >= 0)
    {
        jobject            jobj;
        CSjvmSkinWnd    *pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->newJObject(TID_SKIN_WND, jobj);
        assert(pJObjSkinWnd);
        pJObjSkinWnd->m_pSkinWnd = pWnd;
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnCreate, TID_SKIN_WND, jobj);
    }
}

void SjvmSkinWndEvent::onDestroy()
{
    if (m_sOnDestroy >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnDestroy);
    }
}

void SjvmSkinWndEvent::onSize(int cx, int cy)
{
    if (m_sOnSize >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnSize, TID_INT, cx, TID_INT, cy);
    }
}

void SjvmSkinWndEvent::onMove(int x, int y)
{
    if (m_sOnMove >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnMove, TID_INT, x, TID_INT, y);
    }
}

void SjvmSkinWndEvent::onActivate(bool bActived)
{
    if (m_sOnActivate >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnActivate, TID_BOOLEAN, bActived);
    }
}

void SjvmSkinWndEvent::onMouseActive(bool bMouseActive)
{
    if (m_sOnMouseActive >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnMouseActive, TID_BOOLEAN, bMouseActive);
    }
}

void SjvmSkinWndEvent::onTimer(int nTimerId)
{
    if (m_sOnTimer >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_sOnTimer, TID_INT, nTimerId);
    }
}

SjvmSkinWndEvent::~SjvmSkinWndEvent()
{
    g_sjvm->jniReleaseJObjRef(m_jobject);

    for (size_t i = 0; i < m_vSkinWnds.size(); i++)
        m_vSkinWnds[i]->sjvmSetNotifyEvent(nullptr);
    m_vSkinWnds.clear();
}

void SjvmSkinWndEvent::addSkinWnd(CSkinWnd *pWnd)
{
    m_vSkinWnds.push_back(pWnd);
}

void SjvmSkinWndEvent::removeSkinWnd(CSkinWnd *pWnd)
{
    for (size_t i = 0; i < m_vSkinWnds.size(); i++)
    {
        if (m_vSkinWnds[i] == pWnd)
        {
            m_vSkinWnds.erase(m_vSkinWnds.begin() + i);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

IMPLENENT_JNI_OBJ(CSjvmUIObject, "UIObject")

void CSjvmUIObject::registerJNI()
{
}


//////////////////////////////////////////////////////////////////////////


class CSjvmSkinSystemEventObj : public CJObject
{
    DECLARE_JNI_OBJ(CSjvmSkinSystemEventObj)
public:
    CSjvmSkinSystemEventObj() : CJObject(m_sTypeId, 0) { }

public:
    static void SJNI_API onInit() { }
    static void SJNI_API onDestory() { }
    static void SJNI_API onSkinWndCreate(jobject skinWnd) { }
    static void SJNI_API onSkinWndDestory(jobject skinWnd) { }

    static void registerJNI()
    {
        int        nRet;

        // add class members definition.
        CClass *pClass = g_sjvm->getClass(CLASS_NAME);
        assert(pClass);

        pClass->addNativeDefaultConstructor();

        CSjvmSkinSystemEvent::m_onInit = nRet = pClass->addNativeMethod(false, "onInit", (void *)onInit, TID_VOID, TID_INVALID);
        assert(nRet != -1);

        CSjvmSkinSystemEvent::m_onDestory= nRet = pClass->addNativeMethod(false, "onDestory", (void *)onDestory, TID_VOID, TID_INVALID);
        assert(nRet != -1);

        CSjvmSkinSystemEvent::m_onSkinWndCreate = nRet = pClass->addNativeMethod(false, "onSkinWndCreate", (void *)onSkinWndCreate, TID_VOID, TID_SKIN_WND, TID_INVALID);
        assert(nRet != -1);

        CSjvmSkinSystemEvent::m_onSkinWndDestory = nRet = pClass->addNativeMethod(false, "onSkinWndDestory", (void *)onSkinWndDestory, TID_VOID, TID_SKIN_WND, TID_INVALID);
        assert(nRet != -1);
    }

};

IMPLENENT_JNI_OBJ(CSjvmSkinSystemEventObj, "SkinSystemEvent")

//////////////////////////////////////////////////////////////////////////

int        CSjvmSkinSystemEvent::m_onInit = 0;
int        CSjvmSkinSystemEvent::m_onDestory = 0;
int        CSjvmSkinSystemEvent::m_onSkinWndCreate = 0;
int        CSjvmSkinSystemEvent::m_onSkinWndDestory = 0;

void CSjvmSkinSystemEvent::onInit()
{
    if (m_onInit >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_onInit);
    }
}

void CSjvmSkinSystemEvent::onDestory()
{
    if (m_onDestory >= 0)
    {
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_onDestory);
    }
}

void CSjvmSkinSystemEvent::onSkinWndCreate(CSkinWnd *pSkinWnd)
{
    if (m_onSkinWndCreate >= 0)
    {
        jobject            jobj;
        CSjvmSkinWnd    *pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->newJObject(TID_SKIN_WND, jobj);
        assert(pJObjSkinWnd);
        pJObjSkinWnd->m_pSkinWnd = pSkinWnd;
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_onSkinWndCreate, TID_SKIN_WND, jobj);
    }
}

void CSjvmSkinSystemEvent::onSkinWndDestory(CSkinWnd *pSkinWnd)
{
    if (m_onSkinWndDestory >= 0)
    {
        jobject            jobj;
        CSjvmSkinWnd    *pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->newJObject(TID_SKIN_WND, jobj);
        assert(pJObjSkinWnd);
        pJObjSkinWnd->m_pSkinWnd = pSkinWnd;
        g_sjvm->jniCallMethod(TID_INVALID, m_jobject, m_onSkinWndDestory, TID_SKIN_WND, jobj);
    }
}

//////////////////////////////////////////////////////////////////////////


IMPLENENT_JNI_OBJ(CSjvmSkinSystem, "SkinSystem")

ListSjvmSkinWndEvent    CSjvmSkinSystem::m_listSkinWndEvent;
ListSkinSystemEvent        CSjvmSkinSystem::m_listSkinSystemEvent;
CSkinFactory            *CSjvmSkinSystem::m_pSkinFactory = nullptr;

CSjvmSkinSystem::CSjvmSkinSystem() : CJObject(m_sTypeId, 0)
{

}

void CSjvmSkinSystem::init(CSkinFactory *pSkinFactory)
{
    assert(g_sjvm == nullptr);
    g_sjvm = new CSJVM();

    m_pSkinFactory = pSkinFactory;

    initSjvmLib(g_sjvm);

    // register class name
    CSjvmSkinWnd::registerClassType(g_sjvm);
    CSjvmSkinSystem::registerClassType(g_sjvm);
    CSjvmSkinSystemEventObj::registerClassType(g_sjvm);
    CSjvmSkinWndEventObj::registerClassType(g_sjvm);

    // add class members definition.
    CSjvmSkinWnd::registerJNI();
    CSjvmSkinSystem::registerJNI();
    CSjvmSkinSystemEventObj::registerJNI();
    CSjvmSkinWndEventObj::registerJNI();

    // compile system module files

    // set flag to freeze the current SJVM status as system module.
    g_sjvm->finishedInitSysModule();
}

void CSjvmSkinSystem::quit()
{
    delete g_sjvm;
    g_sjvm = nullptr;
}

void CSjvmSkinSystem::loadScript(cstr_t szScript)
{
    VecStrings        vFiles;
    int            nRet;

    strSplit(szScript, ',', vFiles);
    for (int i = 0; i < (int)vFiles.size(); i++)
    {
        trimStr(vFiles[i]);

        string file;
        if (!m_pSkinFactory->getResourceMgr()->getResourcePathName(vFiles[i].c_str(), file))
        {
            ERR_LOG1("Script files does NOT exist: %s", vFiles[i].c_str());
            return;
        }

        vFiles[i] = file;
    }

    nRet = g_sjvm->compileFiles(vFiles);
    if (nRet != ERR_OK)
    {
        ERR_LOG2("Failed to compile script files: %s, error: %s", szScript, (cstr_t)Error2Str(nRet));
        return;
    }

    //
    // init and run script
    //

    g_sjvm->run();

    for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
        it != m_listSkinSystemEvent.end(); ++it)
    {
        CSjvmSkinSystemEvent *pEvent = *it;
        pEvent->onInit();
    }
}

void CSjvmSkinSystem::closeScript()
{
    for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
        it != m_listSkinSystemEvent.end(); ++it)
    {
        CSjvmSkinSystemEvent *pEvent = *it;
        pEvent->onDestory();
    }

    m_listSkinSystemEvent.clear();
    m_listSkinWndEvent.clear();

    g_sjvm->reset();
}

void CSjvmSkinSystem::onSkinWndCreate(CSkinWnd *pSkinWnd)
{
    // Notify onSkinWndCreate
    for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
        it != m_listSkinSystemEvent.end(); ++it)
    {
        CSjvmSkinSystemEvent *pEvent = *it;
        pEvent->onSkinWndCreate(pSkinWnd);
    }

    // register notify of SjvmSkinWndEvent
    for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
        it != m_listSkinWndEvent.end(); ++it)
    {
        SjvmSkinWndEvent    *pEvent = *it;
        if (strcmp(pEvent->m_strSkinWndName.c_str(), pSkinWnd->getSkinWndName()) == 0)
        {
            pSkinWnd->sjvmSetNotifyEvent(pEvent);
            pEvent->addSkinWnd(pSkinWnd);
            pEvent->onCreate(pSkinWnd);
        }
    }
}

void CSjvmSkinSystem::onSkinWndDestory(CSkinWnd *pSkinWnd)
{
    // Notify onSkinWndDestory
    for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
        it != m_listSkinSystemEvent.end(); ++it)
    {
        CSjvmSkinSystemEvent *pEvent = *it;
        pEvent->onSkinWndDestory(pSkinWnd);
    }

    // Unregister notify of SjvmSkinWndEvent
    for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
        it != m_listSkinWndEvent.end(); ++it)
    {
        SjvmSkinWndEvent    *pEvent = *it;
        if (strcmp(pEvent->m_strSkinWndName.c_str(), pSkinWnd->getSkinWndName()) == 0)
        {
            pSkinWnd->sjvmSetNotifyEvent(nullptr);
            pEvent->removeSkinWnd(pSkinWnd);
        }
    }
}

void CSjvmSkinSystem::registerJNI()
{
    int        nRet;

    // add class members definition.
    CClass *pClass = g_sjvm->getClass(CLASS_NAME);
    assert(pClass);

    pClass->addNativeDefaultConstructor();

    nRet = pClass->addNativeMethod(true, "registerSkinSystemEvent", (void *)registerSkinSystemEvent, TID_BOOLEAN, CSjvmSkinSystemEventObj::m_sTypeId, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "unRegisterSkinSystemEvent", (void *)unRegisterSkinSystemEvent, TID_VOID, CSjvmSkinSystemEventObj::m_sTypeId, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "registerSkinWndEvent", (void *)registerSkinWndEvent, TID_BOOLEAN, TID_STRING, TID_SKIN_WND_NOTIFY_EVENT, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "unRegisterSkinWndEvent", (void *)unRegisterSkinWndEvent, TID_VOID, TID_SKIN_WND_NOTIFY_EVENT, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "getSkinWnd", (void *)getSkinWnd, TID_VOID, TID_SKIN_WND, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "getCommandID", (void *)getCommandID, TID_INT, TID_STRING, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "writeProfileInt", (void *)writeProfileInt, TID_VOID, TID_STRING, TID_INT, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "writeProfileString", (void *)writeProfileString, TID_VOID, TID_STRING, TID_STRING, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "getProfileInt", (void *)getProfileInt, TID_INT, TID_STRING, TID_INT, TID_INVALID);
    assert(nRet != -1);

    nRet = pClass->addNativeMethod(true, "getProfileString", (void *)getProfileString, TID_STRING, TID_STRING, TID_STRING, TID_INVALID);
    assert(nRet != -1);
}

bool SJNI_API CSjvmSkinSystem::registerSkinSystemEvent(jobject event)
{
    assert(!(event == JOBJ_NULL));
    if (event == JOBJ_NULL)
        return false;

    CSjvmSkinSystemEvent    *pEvent = new CSjvmSkinSystemEvent();

    g_sjvm->jniAddJObjRef(event);
    pEvent->m_jobject = event;
    m_listSkinSystemEvent.push_back(pEvent);

    return true;
}

void SJNI_API CSjvmSkinSystem::unRegisterSkinSystemEvent(jobject event)
{
    // Is it exist ?
    for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
        it != m_listSkinSystemEvent.end(); ++it)
    {
        CSjvmSkinSystemEvent *pEvent = *it;
        if (pEvent->m_jobject == event)
        {
            g_sjvm->jniReleaseJObjRef(event);
            m_listSkinSystemEvent.erase(it);
            delete pEvent;
            return;
        }
    }

    ERR_LOG1("SimpleJVM:: Not found registered SjvmSkinSystemEvent: %d", event);
}

bool SJNI_API CSjvmSkinSystem::registerSkinWndEvent(cstr_t szWndName, jobject event)
{
    assert(!(szWndName == nullptr || event == JOBJ_NULL));
    if (szWndName == nullptr || event == JOBJ_NULL)
        return false;

    SjvmSkinWndEvent    *pEvent = new SjvmSkinWndEvent();

    g_sjvm->jniAddJObjRef(event);

    pEvent->m_jobject = event;
    pEvent->m_strSkinWndName = szWndName;
    m_listSkinWndEvent.push_back(pEvent);

    return true;
}

void SJNI_API CSjvmSkinSystem::unRegisterSkinWndEvent(jobject event)
{
    // Is it exist ?
    for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
        it != m_listSkinWndEvent.end(); ++it)
    {
        SjvmSkinWndEvent    *pEvent = *it;
        if (pEvent->m_jobject == event)
        {
            g_sjvm->jniReleaseJObjRef(event);
            m_listSkinWndEvent.erase(it);
            delete pEvent;
            return;
        }
    }

    ERR_LOG1("SimpleJVM:: Not found registered SjvmSkinWndEvent: %d", event);
}

jobject SJNI_API CSjvmSkinSystem::getSkinWnd(cstr_t skinWndName)
{
    assert(m_pSkinFactory);
    CSkinWnd *pWnd = m_pSkinFactory->findSkinWnd(skinWndName);
    if (!pWnd)
        return JOBJ_NULL;

    jobject        jobj;
    CSjvmSkinWnd *pJObj = (CSjvmSkinWnd*)g_sjvm->newJObject(TID_SKIN_WND, jobj);
    assert(pJObj && pJObj->m_typeId == CSjvmSkinWnd::m_sTypeId);

    pJObj->m_pSkinWnd = pWnd;

    return jobj;
}

int SJNI_API CSjvmSkinSystem::getCommandID(cstr_t string)
{
    assert(m_pSkinFactory);

    return m_pSkinFactory->getIDByName(string);
}


void SJNI_API CSjvmSkinSystem::writeProfileInt(cstr_t szValueName, int value)
{
    g_profile.writeInt(m_pSkinFactory->getSkinName(), szValueName, value);
}

void SJNI_API CSjvmSkinSystem::writeProfileString(cstr_t szValueName, cstr_t szValue)
{
    g_profile.writeString(m_pSkinFactory->getSkinName(), szValueName, szValue);
}

int SJNI_API CSjvmSkinSystem::getProfileInt(cstr_t szValueName, int nDefault)
{
    return g_profile.getInt(m_pSkinFactory->getSkinName(), szValueName, nDefault);
}

jobject SJNI_API CSjvmSkinSystem::getProfileString(cstr_t szValueName, cstr_t szDefault)
{
    cstr_t szValue = g_profile.getString(m_pSkinFactory->getSkinName(), szValueName, szDefault);

    if (!szValue)
        return g_sjvm->newString("", false);

    return g_sjvm->newString(szValue, true);
}
