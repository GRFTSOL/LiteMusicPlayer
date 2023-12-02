#pragma once

class CMPPlaylistCmdHandler : public ISkinCmdHandler {
public:
    CMPPlaylistCmdHandler();
    virtual ~CMPPlaylistCmdHandler();

    // if the command id is processed, return true.
    virtual bool onCommand(uint32_t nId);
    virtual bool onUIObjNotify(IUIObjNotify *pNotify);

};
