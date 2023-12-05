#pragma once

#include "Profile.h"
#include "Log.h"


extern class CProfile g_profile;
extern class CLog     g_log;

bool initBaseFramework(int argc, const char *argv[], cstr_t logFile, cstr_t profileName, cstr_t defAppName);

void setAppResourceDir(cstr_t path);
void setAppDataDir(cstr_t path);

const string &getAppResourceDir();
string getAppResourceFile(cstr_t name);

const string &getAppDataDir();
string getAppDataFile(cstr_t name);

#ifdef _WIN32
HINSTANCE getAppInstance();
void setAppInstance(HINSTANCE instance);
#endif
