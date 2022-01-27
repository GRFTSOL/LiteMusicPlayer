#pragma once

class CMPPlaylistCmdHandler : public ISkinCmdHandler  
{
public:
    CMPPlaylistCmdHandler();
    virtual ~CMPPlaylistCmdHandler();
    
    // if the command id is processed, return true.
    virtual bool onCommand(int nId);
    virtual bool onCustomCommand(int nId);
    virtual bool onUIObjNotify(IUIObjNotify *pNotify);

};
