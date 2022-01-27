; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "mpskineditor.h"
LastPage=0

ClassCount=18
Class1=CChildFrame
Class2=CChildView
Class3=CDlgChooseIDs
Class4=CDlgImageProperty
Class5=CDlgOpenSkin
Class6=CDynCmdFrame
Class7=CDynCtrlFrame
Class8=CImageRectSelCtrl
Class9=CMainFrame
Class10=CMDITabs
Class11=CMPSkinEditorApp
Class12=CAboutDlg
Class13=CObjListBar
Class14=CPropertyBar
Class15=CComboButton
Class16=CSEPropertyListCtrl
Class17=CSizingControlBar
Class18=CSkinEditorFrame

ResourceCount=9
Resource1=IDD_ABOUTBOX (English (U.S.))
Resource2=IDR_DYNCMD (English (U.S.))
Resource3=IDR_MPSKINTYPE (English (U.S.))
Resource4=IDR_MAINFRAME (English (U.S.))
Resource5=IDR_TEMP
Resource6=IDD_OPEN_SKIN
Resource7=IDD_IMAGE_PROPERTY
Resource8=IDD_CHOOSE_ID (English (U.S.))
Resource9=IDR_CONTEXT

[CLS:CChildFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp

[CLS:CChildView]
Type=0
BaseClass=CWnd
HeaderFile=ChildView.h
ImplementationFile=ChildView.cpp

[CLS:CDlgChooseIDs]
Type=0
BaseClass=CDialog
HeaderFile=DlgChooseIDs.h
ImplementationFile=DlgChooseIDs.cpp

[CLS:CDlgImageProperty]
Type=0
BaseClass=CDialog
HeaderFile=DlgImageProperty.h
ImplementationFile=DlgImageProperty.cpp

[CLS:CDlgOpenSkin]
Type=0
BaseClass=CDialog
HeaderFile=DlgOpenSkin.h
ImplementationFile=DlgOpenSkin.cpp

[CLS:CDynCmdFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=DynCmdFrame.h
ImplementationFile=DynCmdFrame.cpp
Filter=W
VirtualFilter=mfWC

[CLS:CDynCtrlFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=DynCtrlFrame.h
ImplementationFile=DynCtrlFrame.cpp
Filter=W
VirtualFilter=mfWC

[CLS:CImageRectSelCtrl]
Type=0
BaseClass=CStatic
HeaderFile=ImageRectSel.h
ImplementationFile=ImageRectSel.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CMDIFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=W
VirtualFilter=fWC
LastObject=IDC_SKIN_WND_DEL

[CLS:CMDITabs]
Type=0
BaseClass=CTabCtrl
HeaderFile=MDITabs.h
ImplementationFile=MDITabs.cpp

[CLS:CMPSkinEditorApp]
Type=0
BaseClass=CWinApp
HeaderFile=MPSkinEditor.h
ImplementationFile=MPSkinEditor.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=MPSkinEditor.cpp
ImplementationFile=MPSkinEditor.cpp
LastObject=IDC_NEW_SKIN_WND

[CLS:CObjListBar]
Type=0
BaseClass=CSizingControlBar
HeaderFile=ObjListBar.h
ImplementationFile=ObjListBar.cpp

[CLS:CPropertyBar]
Type=0
BaseClass=CSizingControlBar
HeaderFile=PropertyBar.h
ImplementationFile=PropertyBar.cpp

[CLS:CComboButton]
Type=0
BaseClass=CButton
HeaderFile=SEPropertyListCtrl.h
ImplementationFile=SEPropertyListCtrl.cpp

[CLS:CSEPropertyListCtrl]
Type=0
BaseClass=CListBox
HeaderFile=SEPropertyListCtrl.h
ImplementationFile=SEPropertyListCtrl.cpp

[CLS:CSizingControlBar]
Type=0
BaseClass=baseCSizingControlBar
HeaderFile=sizecbar.h
ImplementationFile=sizecbar.cpp

[CLS:CSkinEditorFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=SkinEditorFrame.h
ImplementationFile=SkinEditorFrame.cpp
Filter=W
VirtualFilter=mfWC

[DLG:IDD_CHOOSE_ID]
Type=1
Class=CDlgChooseIDs

[DLG:IDD_IMAGE_PROPERTY]
Type=1
Class=CDlgImageProperty
ControlCount=26
Control1=IDOK,button,1342373889
Control2=IDCANCEL,button,1342242816
Control3=IDC_IMAGE,static,1350570251
Control4=IDC_S_CURSOR,static,1342308352
Control5=IDC_S_FILE,static,1342308352
Control6=IDC_CB_IMAGE,combobox,1344339971
Control7=IDC_S_X,static,1342308352
Control8=IDC_E_X,edit,1484849280
Control9=IDC_S_CX,static,1342308352
Control10=IDC_E_CX,edit,1484849280
Control11=IDC_S_Y,static,1342308352
Control12=IDC_E_Y,edit,1484849280
Control13=IDC_S_CY,static,1342308352
Control14=IDC_E_CY,edit,1484849280
Control15=IDC_S_SSX,static,1342308352
Control16=IDC_E_STRETCH_START,edit,1484849280
Control17=IDC_S_SEX,static,1342308352
Control18=IDC_E_STRETCH_END,edit,1484849280
Control19=IDC_S_SSX2,static,1342308352
Control20=IDC_E_STRETCH_START2,edit,1484849280
Control21=IDC_S_SEX2,static,1342308352
Control22=IDC_E_STRETCH_END2,edit,1484849280
Control23=IDC_ROOM_OUT,button,1342242816
Control24=IDC_ROOM_IN,button,1342242816
Control25=IDC_UPDATE_RC,button,1073807360
Control26=IDC_S_IMG_SIZE,static,1342308352

[DLG:IDD_OPEN_SKIN]
Type=1
Class=CDlgOpenSkin
ControlCount=7
Control1=IDOK,button,1342373889
Control2=65535,static,1342308352
Control3=IDCANCEL,button,1342242816
Control4=IDC_E_SKIN_ROOT_DIR,edit,1350631552
Control5=IDC_BR_SKIN_ROOT_DIR,button,1342242816
Control6=IDC_L_SKINS,listbox,1352728835
Control7=IDC_E_INFO,edit,1353777348

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg

[MNU:IDR_CONTEXT]
Type=1
Class=?
Command1=ID_EDIT_CUT
Command2=IDC_DELETE
Command3=ID_EDIT_COPY
Command4=ID_EDIT_PASTE
Command5=IDC_UNDO
Command6=IDC_REDO
CommandCount=6

[MNU:IDR_TEMP]
Type=1
Class=?
Command1=IDC_PROPERTY
Command2=IDC_PROPERTY_SAVE
Command3=IDC_UPDATE_SKIN_OBJ_LIST
Command4=IDC_SKIN_OBJ_LIST_SEL_CHANGED
Command5=IDC_UPDATE_DATA_TO_XML
CommandCount=5

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=IDC_NEW_SKIN
Command2=IDC_OPEN_SKIN
Command3=ID_APP_EXIT
Command4=ID_VIEW_TOOLBAR
Command5=ID_VIEW_STATUS_BAR
Command6=IDC_VIEW_PROPERTY_BAR
Command7=IDC_VIEW_OBJLIST_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_MPSKINTYPE (English (U.S.))]
Type=1
Class=CMainFrame
Command1=IDC_NEW_SKIN
Command2=IDC_OPEN_SKIN
Command3=IDC_SAVE_SKIN
Command4=IDC_CLOSE_SKIN
Command5=ID_APP_EXIT
Command6=IDC_UNDO
Command7=IDC_REDO
Command8=ID_EDIT_CUT
Command9=IDC_DELETE
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=IDC_SKIN_WND_NEW
Command13=IDC_SKIN_WND_DEL
Command14=IDC_SKIN_WND_DUP
Command15=IDC_INSERT_BTN
Command16=IDC_INSERT_FOCUS_BTN
Command17=IDC_INSERT_NSTATEBTN
Command18=IDC_INSERT_CAPTION_IMG
Command19=IDC_INSERT_IMG
Command20=IDC_INSERT_FOCUS_IMG
Command21=IDC_INSERT_XSCALE_IMG
Command22=IDC_INSERT_YSCALE_IMG
Command23=IDC_INSERT_HSCROLLBAR
Command24=IDC_INSERT_VSCROLLBAR
Command25=IDC_RGNCREATOR
Command26=ID_VIEW_TOOLBAR
Command27=ID_VIEW_STATUS_BAR
Command28=IDC_VIEW_PROPERTY_BAR
Command29=IDC_VIEW_OBJLIST_BAR
Command30=IDC_ROOM_IN
Command31=IDC_ROOM_OUT
Command32=ID_WINDOW_CASCADE
Command33=ID_WINDOW_TILE_HORZ
Command34=ID_WINDOW_ARRANGE
Command35=ID_APP_ABOUT
CommandCount=35

[MNU:IDR_DYNCMD (English (U.S.))]
Type=1
Class=?
Command1=IDC_NEW_SKIN
Command2=IDC_OPEN_SKIN
Command3=IDC_SAVE_SKIN
Command4=IDC_CLOSE_SKIN
Command5=ID_APP_EXIT
Command6=IDC_UNDO
Command7=IDC_REDO
Command8=ID_EDIT_CUT
Command9=IDC_DELETE
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_VIEW_TOOLBAR
Command13=ID_VIEW_STATUS_BAR
Command14=IDC_VIEW_PROPERTY_BAR
Command15=IDC_VIEW_OBJLIST_BAR
Command16=IDC_ROOM_IN
Command17=IDC_ROOM_OUT
Command18=ID_WINDOW_CASCADE
Command19=ID_WINDOW_TILE_HORZ
Command20=ID_WINDOW_ARRANGE
Command21=ID_APP_ABOUT
CommandCount=21

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_CHOOSE_ID (English (U.S.))]
Type=1
Class=?
ControlCount=3
Control1=IDOK,button,1342373889
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST,SysListView32,1350631437

[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_EDIT_CUT
Command3=ID_EDIT_COPY
Command4=ID_EDIT_PASTE
Command5=ID_FILE_PRINT
Command6=ID_APP_ABOUT
CommandCount=6

[ACL:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_EDIT_PASTE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_NEXT_PANE
Command7=ID_PREV_PANE
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_EDIT_CUT
Command11=ID_EDIT_UNDO
CommandCount=11

