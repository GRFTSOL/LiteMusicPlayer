#pragma once

class CMPlayerApp : public CMPlayerAppBase {
public:
    CMPlayerApp();
    virtual ~CMPlayerApp();

    bool init(int argc, char *argv[]);

    void quit();

    bool isRunning();

protected:

};
