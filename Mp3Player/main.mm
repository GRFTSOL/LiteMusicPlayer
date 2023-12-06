//
//  main.m
//
//  Created by Hongyong Xiao on 11/13/11.
//

#import <Cocoa/Cocoa.h>
#import "../MPlayerUI/MPlayerApp.h"
#include "utils/unittest.h"


int main(int argc, const char *argv[])
{
    initUnittest(argc, argv);

//    InitBaseFrameWork(argc, argv, "MiniLyrics.log", "MiniLyrics.ini", SZ_SECT_UI);
    initBaseFramework(argc, (const char **)argv, "MP3Player.log",
        "/Users/henry_xiao/ProjectsPrivate/Mp3Player/build/Debug/MP3Player.ini", "MP3Player");

    return NSApplicationMain(argc, (const char **)argv);
}
//
//#import <Cocoa/Cocoa.h>
//#import "../Window/WindowLib.h"
//#import "../Skin/SkinTypes.h"
//#import "../Skin/Skin.h"
//#import "MPlayerApp.h"
//#import <Metal/MTLDevice.h>
//
//
//int main(int argc, const char * argv[]) {
//    @autoreleasepool {
//        // Setup code that might create autoreleased objects goes here.
//    }
//
//    initBaseFramework(argc, argv, "MP3Player.log", "/Users/hongyongxiao/ProjectsPrivate/Mp3Player/build/Debug/MP3Player.ini", "MP3Player");
//    // InitBaseFrameWork(argc, argv, "MiniLyrics.log", "MiniLyrics.ini", SZ_SECT_UI);
//
////    auto player = MPlayerApp::getInstance();
////
////    player->init();
////
////    Window *wnd = new Window();
////    wnd->createForSkin("x", "abc", 10, 10, 300, 200, nullptr, false, false, true);
//
//    return NSApplicationMain(argc, argv);
//}
