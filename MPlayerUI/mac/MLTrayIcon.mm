#import <AppKit/AppKit.h>

@interface _StatusbarCtrl : NSObject
{
    uint32_t                cmdId;
}

- (void)onClicked;

@end

#include "MLTrayIcon.h"
#include "MLCmd.h"
#include "MPlayerApp.h"
#include "../MPFloatingLyrWnd.h"


@implementation _StatusbarCtrl

- (id)init: (uint32_t)cmdId {
    self = [super init];

    self->cmdId = cmdId;

    return self;
}

- (void)onClicked {
    if (self->cmdId != UID_INVALID) {
        if (g_wndFloatingLyr.isValid()) {
            g_wndFloatingLyr.m_ignoreOneActivate = true;
        }

        CMPlayerApp::getInstance()->getMainWnd()->postCustomCommandMsg(self->cmdId);
    }
}

@end


#define STATUS_ITEM_VIEW_WIDTH 24.0

SYSTRAY_ICON_CMD    g_sysTrayIconCmd[] = {
    { CMD_NEXT,    _TLM("Next Track"), true },
    { CMD_PLAYPAUSE, _TLM("Play/pause"), true },
    { CMD_PREVIOUS, _TLM("Previous Track"), false },
    { UID_INVALID, _TLM("Menu"), true },
};

int MAX_PLAYER_TRAY_ICON_CMD = CountOf(g_sysTrayIconCmd);

NSString *ICON_PLAY = @"play", *ICON_PLAY_DISABLED = @"play-disabled";
NSString *ICON_PAUSE = @"pause";
NSString *ICON_NEXT = @"next", *ICON_NEXT_DISABLED = @"next-disabled";
NSString *ICON_PREV = @"prev", *ICON_PREV_DISABLED = @"prev-disabled";
NSString *ICON_MENU = @"menu-logo";


struct TrayItem {
    NSStatusItem            *statusItem = nullptr;
    CMenu                   *menu = nullptr;
    cstr_t                  iconName = nullptr;
};

using VecTrayItems = std::vector<TrayItem>;

struct MLtrayIconPrivate {
    TrayItem                items[CountOf(g_sysTrayIconCmd)];
};

NSString *getStatusItemIcon(uint32_t cmd) {
    if (cmd == UID_INVALID) {
        return ICON_MENU;
    }

    auto playlist = g_player.getCurrentPlaylist();
    if (!playlist || playlist->getCount() == 0) {
        if (cmd == CMD_PLAYPAUSE) {
            return ICON_PLAY_DISABLED;
        } else if (cmd == CMD_NEXT) {
            return ICON_NEXT_DISABLED;
        } else {
            return ICON_PREV_DISABLED;
        }
    }

    if (cmd == CMD_PLAYPAUSE) {
        if (g_player.getPlayerState() == PS_PLAYING) {
            return ICON_PAUSE;
        }
        return ICON_PLAY;
    } else if (cmd == CMD_NEXT) {
        return ICON_NEXT;
    } else {
        return ICON_PREV;
    }
}

CMLTrayIcon::CMLTrayIcon() {
    _data = new MLtrayIconPrivate;
}

CMLTrayIcon::~CMLTrayIcon() {
    delete _data;
}

void CMLTrayIcon::init(Window *pWnd) {
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        string key = stringPrintf("TrayIcon%d", i);
        g_sysTrayIconCmd[i].isEnabled = g_profile.getBool(key.c_str(), g_sysTrayIconCmd[i].isEnabled);
    }

    updatePlayerSysTrayIcon();
}

void CMLTrayIcon::quit() {

}

void CMLTrayIcon::updateShowIconPos() {

}

void CMLTrayIcon::updatePlayerSysTrayIcon() {
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        auto &setting = g_sysTrayIconCmd[i];
        auto &item = _data->items[i];

        if (setting.isEnabled) {
            if (item.statusItem == nullptr) {
                // 未创建
                NSStatusItem *statusItem = [[NSStatusBar systemStatusBar]
                                            statusItemWithLength: STATUS_ITEM_VIEW_WIDTH];
                [statusItem retain];
                item.statusItem = statusItem;

                [statusItem setImage:[NSImage imageNamed:getStatusItemIcon(setting.cmdId)]];
                if (setting.cmdId == UID_INVALID) {
                    // Menu
                    CMPlayerApp::getInstance()->getSkinFactory()->loadMenu(CMPlayerApp::getMainWnd(), &item.menu, "TrayIconMenu");

                    [statusItem setMenu:(NSMenu *)item.menu->getHandle(CMPlayerApp::getMainWnd())];
                } else {
                    auto statusCtrl = [[_StatusbarCtrl alloc] init:setting.cmdId];
                    auto button = [statusItem button];
                    [button setTarget:statusCtrl];
                    [button setAction:@selector(onClicked)];
                }
            } else {
                [item.statusItem setImage:[NSImage imageNamed:getStatusItemIcon(setting.cmdId)]];
            }
        } else {
            if (item.statusItem != nullptr) {
                // Destroy
                [item.statusItem release];
                item.statusItem = nullptr;
            }
        }
    }
}

void CMLTrayIcon::forceShow(bool bForceShow) {

}

void CMLTrayIcon::updateTrayIconText(cstr_t text) {
    for (auto &item : _data->items) {
        if (item.statusItem) {
            auto button = [item.statusItem button];
            [button setToolTip:[[NSString alloc] initWithUTF8String:text]];
        }
    }
}
