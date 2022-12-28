#include "../MPlayerApp.h"
#include "DlgChooseColor.h"


CDlgChooseColor::CDlgChooseColor() {
    m_window = nullptr;
    m_clr = RGB(255, 255, 255);
}

CDlgChooseColor::~CDlgChooseColor() {
    if (m_window) {
        gtk_widget_destroy(m_window);
    }
}

int CDlgChooseColor::doModal(Window *pWndParent, const CColor &clr) {
    int nRet;
    GdkColor selClr;

    m_clr = clr;
    m_window = gtk_color_selection_dialog_new("Select Color");

    selClr.red = m_clr.red;
    selClr.green = m_clr.green;
    selClr.blue = m_clr.blue;
    gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(m_window), &selClr);

    nRet = gtk_dialog_run(GTK_DIALOG(m_window));
    if (nRet == IDOK) {
        gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(m_window), &selClr);
        m_clr.red = selClr.red;
        m_clr.green = selClr.green;
        m_clr.blue = selClr.blue;
    }

    return nRet;
}
