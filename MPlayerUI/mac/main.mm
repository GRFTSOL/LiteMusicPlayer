//
//  main.m
//  Mp3Player
//
//  Created by HongyongXiao on 2021/11/11.
//

#import <Cocoa/Cocoa.h>
#import "../Window/WindowLib.h"
#import "../Skin/SkinTypes.h"
#import "../Skin/Skin.h"
#import "MPlayerApp.h"
#import <Metal/MTLDevice.h>


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }

    initBaseFramework(argc, argv, "MP3Player.log", "/Users/hongyongxiao/ProjectsPrivate/Mp3Player/build/Debug/MP3Player.ini", "MP3Player");
    // InitBaseFrameWork(argc, argv, "MiniLyrics.log", "MiniLyrics.ini", SZ_SECT_UI);

    CMPlayerApp *pApp = CMPlayerApp::getInstance();
        pApp->m_appMode = SA_IPOD_LYRICS_DOWNLOADER;

//    auto player = MPlayerApp::getInstance();
//
//    player->init();
//
//    Window *wnd = new Window();
//    wnd->createForSkin("x", "abc", 10, 10, 300, 200, nullptr, false, false, true);

    return NSApplicationMain(argc, argv);
}
