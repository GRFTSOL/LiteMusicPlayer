#pragma once

#ifdef _WIN32
#include "win32/EventsDispatcher.h"
#endif

#ifdef _MAC_OS
#include "mac/EventsDispatcher.h"


#endif


bool initBaseFramework(int argc, const char *argv[], cstr_t logFile, cstr_t profileName, cstr_t defAppName);


class CEventsDispatcher;
class CSkinFactory;

class CSkinApp : public IEventHandler {
public:
    CSkinApp(void);
    virtual ~CSkinApp(void);

    static CSkinApp *getInstance();

    virtual bool init();
    virtual void quit();

    virtual void onEvent(const IEvent *pEvent) { }

    virtual void postQuitMessage();

    virtual int loadDefaultSkin(cstr_t szDefaultSkin, bool bCreateSkinWnd = true);

    string getDefaultSkin();
    void writeDefaultSkin(cstr_t szDefaultSkin);

    CEventsDispatcherBase *getEventPatcher() { return m_pEventDispatcher; }
    CSkinFactory *getSkinFactory() { return m_pSkinFactory; }

    int showDialog(Window *pWndParent, cstr_t szDialogName);

protected:
    virtual CEventsDispatcher *newEventPatcher();
    virtual CSkinFactory *newSkinFactory();

protected:
    static CSkinApp             *m_pInstance;
    CEventsDispatcherBase       *m_pEventDispatcher;
    CSkinFactory                *m_pSkinFactory;

};
