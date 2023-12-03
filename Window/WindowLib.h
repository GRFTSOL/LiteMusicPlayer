#pragma once

#ifndef _HEADER_WIDGET_H_
#define _HEADER_WIDGET_H_

#include "WindowTypes.h"
#include "../Utils/Utils.h"
#include "../GfxRaw/GfxRaw.h"

#ifdef WIN32
#include "win32/Window.h"
#include "win32/FolderDialog.h"
#include "win32/FileSaveDlg.h"
#include "win32/FileOpenDlg.h"
#include "win32/MLMenu.h"
#include "win32/WndResizer.h"
#include "win32/DlgChooseColor.h"
#endif

#ifdef _LINUX_GTK2

#endif


#ifdef _MAC_OS
#include "mac/Cursor.h"
#include "mac/Window.h"
#include "mac/FolderDialog.h"
#include "mac/FileSaveDlg.h"
#include "mac/FileOpenDlg.h"
#include "mac/Menu.h"
#include "mac/WndResizer.h"
#include "mac/DlgChooseColor.h"
#endif

#include "WndDrag.h"
#include "Desktop.h"


#endif // _HEADER_WIDGET_H_
