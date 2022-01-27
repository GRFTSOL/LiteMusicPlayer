#include "stringex_t.h"
#include "CharEncoding.h"
#include "misc.h"

int GetOperationSystemType()
{
    return OPS_MACOSX;
}

bool IsWin9xSystem()
{
    return (GetOperationSystemType() <= OPS_WIN9XMORE);
}

#include <sys/time.h>

uint32_t GetTickCount()
{
    timeval tim;
    gettimeofday(&tim, nullptr);
    return (uint32_t)(tim.tv_sec * 1000 + tim.tv_usec / 1000);
}

bool GetMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict)
{
    NSArray *screens = [NSScreen screens];
    int nMaxSpace = -1;
    for (int i = 0; i < [screens count]; i++)
    {
        NSRect screenVisibleFrame = [[screens objectAtIndex:i] visibleFrame];
        CRect rcScreen;
        rcScreen.top = screenVisibleFrame.origin.y;
        rcScreen.left = screenVisibleFrame.origin.x;
        rcScreen.bottom = rcScreen.top + screenVisibleFrame.size.height;
        rcScreen.right = rcScreen.left + screenVisibleFrame.size.width;
        CRect rc;
        rc.IntersectRect(&rcIn, &rcScreen);
        int nSpace = rc.Width() * rc.Height();
        if (nMaxSpace < nSpace)
        {
            nMaxSpace = nSpace;
            rcRestrict = rcScreen;
        }
    }

    return nMaxSpace != -1;
}

void Sleep(uint32_t dwMilliseconds)
{
    usleep((unsigned int)(dwMilliseconds * 1000));
}

bool ExecuteCmdAndWait(cstr_t szCmd, uint32_t dwTimeOut, uint32_t *pExitCode)
{
    return false;
}

bool CopyTextToClipboard(cstr_t szText)
{
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    [board clearContents];
    [board setString:[NSString stringWithUTF8String:szText] forType:NSStringPboardType];
    return true;
}

bool GetClipBoardText(string &str)
{
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    NSString *ns = [board stringForType:NSStringPboardType];
    if (ns == nil)
        return false;
    str = [ns UTF8String];
    return true;
}
