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

    initBaseFramework(argc, argv, "MusicPlayer.log", "MusicPlayer.ini", SZ_SECT_UI);

    return NSApplicationMain(argc, (const char **)argv);
}
