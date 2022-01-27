#import <Foundation/Foundation.h>
#include "SkinTypes.h"

class CProfile    g_profile;

class CLog        g_log;


static string g_resourceDir;
static string g_dataDir;

const string &getAppResourceDir() {
    if (g_resourceDir.empty()) {
#ifdef _IPHONE
        NSArray *paths = NSSearchPathForDirectoriesInDomains(
                                                             NSDocumentDirectory,
                                                             NSUserDomainMask,
                                                             YES);
        NSString *documentsDir = [paths objectAtIndex:0];
        g_resourceDir.assign((const char *)[documentsDir UTF8String]);
#else
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        g_resourceDir.assign((const char *)[resourcePath UTF8String]);
#endif

        dirStringAddSlash(g_resourceDir);
    }
    return g_resourceDir;
}

const string &getAppDataDir() {
    if (g_dataDir.empty()) {
    }
    return g_dataDir;
}

bool initBaseFramework(int argc, const char *argv[], cstr_t logFile, cstr_t profileName, cstr_t defAppName) {
#ifndef _IPHONE

        NSFileManager* fileManager = [NSFileManager defaultManager];

/*        NSError *error = nil;
        NSURL *supportedDir = [fileManager URLForDirectory:NSApplicationSupportDirectory | NSItemReplacementDirectory inDomain:NSUserDomainMask appropriateForURL:[NSURL URLWithString:@"com.crintsoft.zikiplayer"] create:YES error: &error];

        strcpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), [[[supportedDir filePathURL] absoluteString] UTF8String]);*/

        NSURL* appSupportDir = nil;
        NSArray *urls = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
        if ([urls count] > 0) {
            string componentName("com.crintsoft.");
            if (!isEmptyString(defAppName))
                componentName += defAppName;
            appSupportDir = [[urls objectAtIndex:0] URLByAppendingPathComponent:[NSString stringWithUTF8String:componentName.c_str()]];
            g_dataDir.assign([[appSupportDir path] UTF8String]);
            if ([fileManager fileExistsAtPath:[appSupportDir path]] != YES) {
                [fileManager createDirectoryAtURL:appSupportDir
                      withIntermediateDirectories:YES
                                       attributes:nil
                                            error:nil];
            }

            dirStringAddSlash(g_dataDir);
        }
#endif

    g_profile.init(profileName, defAppName);

    g_log.init(logFile);
    return true;
}
