#pragma once

#ifndef _WIN32
#define IDC_ARROW           32512
#define IDC_IBEAM           32513
#define IDC_WAIT            32514
#define IDC_CROSS           32515
#define IDC_UPARROW         32516
#define IDC_ICON            32641
#define IDC_SIZENWSE        32642
#define IDC_SIZENESW        32643
#define IDC_SIZEWE          32644
#define IDC_SIZENS          32645
#define IDC_SIZEALL         32646
#define IDC_NO              32648
#define IDC_HELP            32651
#define IDC_HAND            32652
#endif

class Cursor {
public:

    enum STD_CURSOR_TYPE {
        C_ARROW                     = (int)(int64_t)IDC_ARROW,
        C_CROSS                     = (int)(int64_t)IDC_CROSS,
        C_SIZEALL                   = (int)(int64_t)IDC_SIZEALL,
        C_SIZENESW                  = (int)(int64_t)IDC_SIZENESW,
        C_SIZENS                    = (int)(int64_t)IDC_SIZENS,
        C_SIZENWSE                  = (int)(int64_t)IDC_SIZENWSE,
        C_SIZEWE                    = (int)(int64_t)IDC_SIZEWE,
        C_NO                        = (int)(int64_t)IDC_NO,
        C_IBEAM                     = (int)(int64_t)IDC_IBEAM,
        C_HAND                      = (int)(int64_t)IDC_HAND,
    };

    Cursor(const Cursor &) = delete;
    Cursor &operator=(const Cursor &) = delete;

    Cursor(void);
    virtual ~Cursor(void);

    bool isValid() const;

    bool loadStdCursor(STD_CURSOR_TYPE cursorType);
    bool loadCursorFromFile(cstr_t szFile);

    void destroy();

    void set();

public:
    struct _CursorInternal       *_data = nullptr;

};
