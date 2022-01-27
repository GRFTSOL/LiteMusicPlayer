// SEPropertyListCtrl.cpp: implementation of the CSEPropertyListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinEditor.h"
#include "SEPropertyListCtrl.h"
#include "DlgImageProperty.h"
#include "DlgChooseIDs.h"

#define IDC_P_EDIT            10
#define IDC_P_BUTTON        3
#define IDC_P_COMB_BT        4
#define IDC_P_COMB_LIST        5

CSESkinPropertyAdapter        g_seSkinPropertyAdapter;

/////////////////////////////////////////////////////////////////////////////
// CComboButton
CComboButton::CComboButton()
{
}

CComboButton::~CComboButton()
{
    // delete the objects created
    delete m_pBkBrush;
    delete m_pBlackBrush;
    delete m_pGrayPen;
    delete m_pBkPen;
}

BEGIN_MESSAGE_MAP(CComboButton, CButton)
    //{{AFX_MSG_MAP(CComboButton)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboButton message handlers
bool CComboButton::create( CRect Rect, CWnd* pParent, uint32_t uID)
{
    // create the Brushes and Pens
    m_pBkBrush        = new CBrush( GetSysColor(COLOR_BTNFACE));
    m_pBkPen        = new CPen( PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    m_pGrayPen        = new CPen( PS_SOLID, 1, RGB(128,128,128));
    m_pBlackBrush    = new CBrush(RGB(0,0,0)); 

    // create the CButton
    if( !CButton::create( "", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_OWNERDRAW, Rect, pParent, uID ))
        return false;
        
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// draw the Button
void CComboButton::drawItem(LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    CDC*    pDC            = CDC::fromHandle(lpDrawItemStruct->hDC);
    CRect     ButtonRect  = lpDrawItemStruct->rcItem;

    // Fill the Background
    CBrush* pOldBrush = (CBrush*)pDC->SelectObject( m_pBkBrush );
    CPen* pOldPen = (CPen*)pDC->SelectObject(m_pBkPen);
    pDC->rectangle(ButtonRect);

    // draw the Correct Border
    if(lpDrawItemStruct->itemState & ODS_SELECTED)
    {
        pDC->DrawEdge(ButtonRect, EDGE_SUNKEN, BF_RECT);
        ButtonRect.left++;
        ButtonRect.right++;
        ButtonRect.bottom++;
        ButtonRect.top++;
    }
    else
        pDC->DrawEdge(ButtonRect, EDGE_RAISED, BF_RECT);

    // draw the Triangle
    ButtonRect.left        += 3;
    ButtonRect.right    -= 4;
    ButtonRect.top        += 5;
    ButtonRect.bottom    -= 5;
    drawTriangle(pDC, ButtonRect);

    // Return what was used
    pDC->SelectObject( pOldPen );
    pDC->SelectObject( pOldBrush );
}

void CComboButton::drawTriangle(CDC* pDC, CRect Rect)
{
    CPoint     ptArray[3];    

    // Figure out the Top left
    ptArray[0].x = Rect.left;
    ptArray[0].y = Rect.top;
    ptArray[1].x = Rect.right;
    ptArray[1].y = Rect.top;
    ptArray[2].x = Rect.right - (Rect.width() / 2);
    ptArray[2].y = Rect.bottom;

    // Select the Brush and draw the triangle
    pDC->SelectObject(m_pBlackBrush);
    pDC->Polygon(ptArray, 3 );
}

void CComboButton::measureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSEPropertyListCtrl::CSEPropertyListCtrl()
{
    m_nWidestItem            = 0;
    m_bDeleteFont            = true;
    m_bBoldSelection        = false;

    m_pBkBrush                = nullptr;
    m_pBkPropertyBrush        = nullptr;
    m_pEditWnd                = nullptr;
    m_pButton                = nullptr;
    m_pComboButton            = nullptr;
    m_pListBox                = nullptr;
    m_pBkHighlightBrush        = nullptr;
    m_pSelectedFont            = nullptr;
    m_pBorderPen            = nullptr;
    m_pCurItem                = nullptr;
    m_pCurFont                = nullptr;
    m_pCurDrawItem            = nullptr;
    m_pTextFont                = nullptr;
    m_pSelectedFont            = nullptr;
    m_pBorderPen            = nullptr;

    m_crBorderColor            = RGB(192,192,192);
    m_crBkColor                = GetSysColor(COLOR_WINDOW);
    m_crPropertyBkColor        = m_crBkColor;
    m_crTextColor            = GetSysColor(COLOR_WINDOWTEXT);
    m_crPropertyTextColor    = m_crTextColor;
    m_crHighlightColor        = GetSysColor(COLOR_HIGHLIGHT);
    m_crTextHighlightColor    = GetSysColor(COLOR_HIGHLIGHTTEXT);
}

CSEPropertyListCtrl::~CSEPropertyListCtrl()
{
    if(m_bDeleteFont)        delete m_pTextFont;

    if(m_pEditWnd)            delete m_pEditWnd;
    if(m_pButton)            delete m_pButton;
    if(m_pListBox)            delete m_pListBox;
    if(m_pComboButton)        delete m_pComboButton;

    if(m_pBkBrush)            delete m_pBkBrush;
    if(m_pBkPropertyBrush)    delete m_pBkPropertyBrush;
    if(m_pBkHighlightBrush) delete m_pBkHighlightBrush;
    if(m_pSelectedFont)        delete m_pSelectedFont;
    if(m_pBorderPen)        delete m_pBorderPen;
}

BEGIN_MESSAGE_MAP(CSEPropertyListCtrl, CListBox)
    //{{AFX_MSG_MAP(CSEPropertyListCtrl)
    ON_WM_CREATE()
    ON_WM_CTLCOLOR_REFLECT()
    ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
    ON_WM_CTLCOLOR()
    ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
    ON_EN_KILLFOCUS( IDC_P_EDIT, OnEditLostFocus )
    ON_EN_CHANGE( IDC_P_EDIT, OnEditChange )
    ON_BN_CLICKED( IDC_P_BUTTON, OnButtonClick )
    ON_BN_CLICKED( IDC_P_COMB_BT, OnComboBoxClick )
    ON_LBN_SELCHANGE(IDC_P_COMB_LIST, OnSelChange)
    ON_LBN_KILLFOCUS(IDC_P_COMB_LIST, OnListboxLostFocus)
    ON_WM_LBUTTONDOWN()
    ON_WM_VSCROLL()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEPropertyListCtrl message handlers
int CSEPropertyListCtrl::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CListBox::onCreate(lpCreateStruct) == -1)
        return -1;

    // create the default font and set it
    m_pTextFont = new CFont();
    m_pTextFont->CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                             "ARIAL");    

    // create the Heading font and set it
    m_pSelectedFont = new CFont();
    m_pSelectedFont->CreateFont( 14, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                             "ARIAL");    

    // create the Border Pen
    m_pBorderPen = new CPen(PS_SOLID, 1, m_crBorderColor);

    // create the Selected Background brush
    m_pBkHighlightBrush = new CBrush(m_crHighlightColor);
    m_pBkBrush            = new CBrush(m_crBkColor);

    // set the row height
    SetItemHeight(-1,16);    
    return 0;
}

HBRUSH CSEPropertyListCtrl::CtlColor(CDC* pDC, uint32_t nCtlColor) 
{    
    return (HBRUSH)m_pBkBrush->GetSafeHandle();
}

HBRUSH CSEPropertyListCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, uint32_t nCtlColor) 
{
    HBRUSH hbr = CListBox::OnCtlColor(pDC, pWnd, nCtlColor);

    if( nCtlColor == CTLCOLOR_EDIT) 
    {
        pDC->setBkColor(m_crPropertyBkColor);
        pDC->setTextColor(m_crPropertyTextColor);
    }

    if(m_pBkPropertyBrush)
        return (HBRUSH)(m_pBkPropertyBrush->GetSafeHandle() );
    else
        return hbr;
}

void CSEPropertyListCtrl::drawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    // Make sure its a valid item
    if( lpDrawItemStruct->itemID == LB_ERR )
        return;
     
      // Obtain the text for this item
    m_csText.empty();
    getText(lpDrawItemStruct->itemID, m_csText);

    // get the drawing DC
    CDC* pDC = CDC::fromHandle(lpDrawItemStruct->hDC);
    
    // set the Current member we are drawing
    if (lpDrawItemStruct->itemID < m_vProperies.size())
        m_pCurDrawItem = &m_vProperies[lpDrawItemStruct->itemID];
    else
        m_pCurDrawItem = nullptr;

    // Obtain the Item Rect
    CRect itemRect(lpDrawItemStruct->rcItem);
    
    // draw This item
    drawItem( pDC, itemRect, lpDrawItemStruct->itemState & ODS_SELECTED);
}

void CSEPropertyListCtrl::measureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
}

void CSEPropertyListCtrl::OnDblclk() 
{
    CPoint    pPoint;
    getCursorPos(&pPoint);

    // Convert this rect to coordinates of the desktop    
    CRect TempRect = m_CurRect;
    MapWindowPoints(GetDesktopWindow(), TempRect);

    // Display the Correct Control
    switch (m_pCurItem->prop.valueType)
    {
    case CUIObjProperty::VT_FONT_NAME:
    case CUIObjProperty::VT_IMAGE:
    case CUIObjProperty::VT_IMG_FILE:
    case CUIObjProperty::VT_ID:
        m_pButton->setFocus();        
        OnButtonClick();
        break;
    case CUIObjProperty::VT_COLOR:
        {
            LOGBRUSH lb;
            m_pCurItem->pBrush->GetLogBrush(&lb);
            CColorDialog colorDialog(lb.lbColor, 0, getParent());
            if(colorDialog.doModal() != IDOK)
                return;
            
            // destroy the Brush and create a new one
            if(m_pCurItem->pBrush) delete m_pCurItem->pBrush;
            m_pCurItem->pBrush = new CBrush(colorDialog.getColor());

            char    szStr[64];
            COLORREF    clr = colorDialog.getColor();
            sprintf(szStr, "%02X%02X%02X", clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);

            m_pCurItem->prop.strValue = szStr;

            // Redraw the Widow (Theres probably a better way)
            RedrawWindow();
            
            // send the message that a property has changed
            onPropertyChanged(getCurSel());
        }
        break;
    }
}

void CSEPropertyListCtrl::OnSelchange() 
{        
    hideControls();

    if (!m_pCurItem)
        return;

    // Display the Correct Control
    CRect TempRect = m_CurRect;
    TempRect.inflateRect(-1,-1);

    if (m_pCurItem->isEdit())
    {
        TempRect.left += 1;
        m_pEditWnd->setWindowText(m_pCurItem->prop.strValue.c_str());
        m_pEditWnd->moveWindow(TempRect);
        m_pEditWnd->showWindow(SW_SHOWNORMAL);
        m_pEditWnd->setFocus();
        //                m_pEditWnd->setSel(0,-1);
    }
    else
    {
        switch (m_pCurItem->prop.valueType)
        {
        case CUIObjProperty::VT_FONT_NAME:
        case CUIObjProperty::VT_IMAGE:
        case CUIObjProperty::VT_IMG_FILE:
        case CUIObjProperty::VT_ID:
            TempRect.left = TempRect.right - 17;
            m_pButton->moveWindow(TempRect);
            m_pButton->showWindow(SW_SHOWNORMAL);
            break;
        case CUIObjProperty::VT_COMB_STR:
        case CUIObjProperty::VT_BOOL_STR:
        case CUIObjProperty::VT_FILE:
            TempRect.left = TempRect.right - 17;
            m_pComboButton->moveWindow(TempRect);
            m_pComboButton->showWindow(SW_SHOWNORMAL);
            
            TempRect.left = m_CurRect.left + 2;
            TempRect.right -= 17;
            if(m_pCurItem->bComboEditable)
            {
                m_pEditWnd->setWindowText(m_pCurItem->prop.strValue.c_str());
                //                    m_pEditWnd->setFocus();
                //                    m_pEditWnd->setSel(0,-1);
                m_pEditWnd->moveWindow(TempRect);
                m_pEditWnd->showWindow(SW_SHOWNORMAL);
                m_pEditWnd->setFocus();
            }
            
            // move the Lsit box
            TempRect.left--;
            TempRect.right += 18;
            TempRect.top = TempRect.bottom;
            
            // set the Bottom height
            if(m_pCurItem->prop.options.size() > 5)
                TempRect.bottom += GetItemHeight(0) * 5;
            else
                TempRect.bottom += GetItemHeight(0) * m_pCurItem->prop.options.size();
            m_pListBox->moveWindow(TempRect);

            // Force the Expansion
            OnComboBoxClick();
            break;
        }

    }
}

void CSEPropertyListCtrl::OnEditLostFocus()
{
    m_pEditWnd->showWindow(SW_HIDE);
    if (!m_pCurItem)
    {
        // send the message that a property has changed
        return;
    }

    // get the text
    CString csText;
    m_pEditWnd->getWindowText(csText);

    // Is the current item a text item
    if (m_pCurItem->isEdit())
    {
        // Did the text change
        if(!m_bChanged)
        {
            return;
        }

        m_pCurItem->prop.strValue = csText;

        // send the message that a property has changed
        onPropertyChanged(getCurSel());
    }
    else
    {
/*
        // get the window that has the focus now
        if (GetFocus() == m_pComboButton || !m_pListBox->getCount())
            return;

        // Did the text change
        if(!m_bChanged)
            return;

        m_pCurItem->prop.strValue = csText;

        // send the message that a property has changed
        onPropertyChanged(getCurSel());

        // Look for this text
        m_bChanged = false;
        if( m_pListBox->FindStringExact(-1,csText) != LB_ERR)
            return;

        // add it and select it
        m_pCurItem->nOptSelected = m_pCurItem->prop.options.push_back(csText);

        // resort the strings is necessary
        if(m_pCurItem->bComboSorted)
        {
            // m_pCurItem->prop.options.sort();
    
            // search the the string and set its positon the selected one
            // m_pCurItem->nOptSelected = m_pCurItem->csProperties.FindString(csText);
        }*/

    }
}

void CSEPropertyListCtrl::OnEditChange()
{
    m_bChanged = true;
}

void CSEPropertyListCtrl::OnButtonClick()
{
    if (!m_pCurItem)
        return;

    switch (m_pCurItem->prop.valueType)
    {
    case CUIObjProperty::VT_ID:
        onChooseIDClick();
        break;
    case CUIObjProperty::VT_IMG_FILE:
    case CUIObjProperty::VT_IMAGE:
        onImagePropertyClick();
        break;
    case CUIObjProperty::VT_FONT_NAME:
        onFontPropertyClick();
        break;
    default:
        assert(0);
        break;
    }
}

void CSEPropertyListCtrl::onChooseIDClick()
{
    CDlgChooseIDs    dlg;

    dlg.m_strID = m_pCurItem->prop.strValue;
    if (dlg.doModal() != IDOK)
        return;

    m_pCurItem->prop.strValue = dlg.m_strID;

    // Redraw
    RedrawWindow();

    // send the message that a property has changed
    onPropertyChanged(getCurSel());
}

void CSEPropertyListCtrl::onImagePropertyClick()
{
    CDlgImageProperty    dlg;

    dlg.m_strFile = m_pCurItem->prop.strValue.c_str();
    dlg.m_vNameRects = m_pCurItem->prop.options;

    if(dlg.doModal() != IDOK)
        return;

    m_pCurItem->prop.options = dlg.m_vNameRects;
    m_pCurItem->prop.strValue = dlg.m_strFile;

    // Redraw
    RedrawWindow();

    // send the message that a property has changed
    onPropertyChanged(getCurSel());
}

void CSEPropertyListCtrl::onFontPropertyClick()
{
    // show the Dialog
    CFontDialog fontDialog(&m_pCurItem->LogFont);
    if(fontDialog.doModal() != IDOK)
        return;
    
    // set the Font data 
    fontDialog.GetCurrentFont(&m_pCurItem->LogFont);
    m_pCurItem->prop.strValue = m_pCurItem->LogFont.lfFaceName;

    // Redraw
    RedrawWindow();

    // send the message that a property has changed
    onPropertyChanged(getCurSel());
}

void CSEPropertyListCtrl::OnComboBoxClick()
{
    // add the items
    m_pListBox->resetContent();

    // loop for all items
    for( int nItem = 0; nItem < m_pCurItem->prop.options.size(); nItem++)
        m_pListBox->addString(m_pCurItem->prop.options[nItem].c_str());

    // Select the correct item
    int        nIndex;
    nIndex = m_pListBox->SelectString(-1, m_pCurItem->prop.strValue.c_str());
    m_pListBox->SetTopIndex(nIndex);
    
    // show the List box
    m_pListBox->showWindow(SW_NORMAL);    
}

void CSEPropertyListCtrl::OnSelChange()
{
    CString        str;
    // set the new current item
    m_pCurItem->nOptSelected = m_pListBox->getCurSel();
    m_pListBox->getText(m_pCurItem->nOptSelected, str);
    m_pCurItem->prop.strValue = str;

    // Hide the Windows
    m_pListBox->showWindow(SW_HIDE);

    if(m_pCurItem->bComboEditable)
        m_pEditWnd->setWindowText(m_pCurItem->prop.strValue.c_str());
    else
        RedrawWindow();

    // send the message that a property has changed
    onPropertyChanged(getCurSel());
    m_pComboButton->setFocus();
}

void CSEPropertyListCtrl::OnListboxLostFocus()
{
    CString        str;
    m_pListBox->getText(m_pCurItem->nOptSelected, str);
    m_pCurItem->prop.strValue = str;

    m_pListBox->showWindow(SW_HIDE);
}

void CSEPropertyListCtrl::onLButtonDown(uint32_t nFlags, CPoint point) 
{
    // is there an item at this point
    bool bOutside;
    uint32_t uItem = ItemFromPoint(point, bOutside);

    // Is this outside the client
    if(bOutside)
        hideControls();
    
    CListBox::onLButtonDown(nFlags, point);
}

void CSEPropertyListCtrl::onVScroll(uint32_t nSBCode, uint32_t nPos, CScrollBar* pScrollBar) 
{
    // Hide the Controls
    hideControls();

    CListBox::onVScroll(nSBCode, nPos, pScrollBar);
}

/////////////////////////////////////////////////////////////////////////////
// GUI User Functions
/////////////////////////////////////////////////////////////////////////////
void CSEPropertyListCtrl::setFont(CFont* pFont)
{
    // delete our font and set our font to theirs
    if(m_pTextFont)        delete m_pTextFont;
    if(m_pSelectedFont) delete m_pSelectedFont;
    m_pTextFont = pFont;
    m_bDeleteFont = false;
    
    // Figure out the text size
    LOGFONT lpLogFont;
    m_pTextFont->GetLogFont(&lpLogFont);

    // set the font and redraw
    CWnd::setFont(m_pTextFont, false);

    // create the heading font with the bold attribute
    lpLogFont.lfWeight = FW_BOLD;
    m_pSelectedFont = new CFont();
    m_pSelectedFont->CreateFontIndirect(&lpLogFont);

    // set the Row height
    SetItemHeight(-1,lpLogFont.lfHeight + 2);

    // ** IMPLEMENT LATER ?? **
    // Recalculate the width Position

}

void CSEPropertyListCtrl::setLineStyle(COLORREF crColor, int nStyle)
{
    // delete the old Pen
    if(m_pBorderPen) delete m_pBorderPen;
    
    // create the brush
    m_pBorderPen = new CPen(nStyle, 1, crColor);
    m_crBorderColor = crColor;
}

void CSEPropertyListCtrl::setBkColor(COLORREF crColor)
{
    // delete the old brush
    if(m_pBkBrush) delete m_pBkBrush;
    
    // create the brush
    m_pBkBrush = new CBrush(crColor);
    m_crBkColor = crColor;
}

void CSEPropertyListCtrl::setPropertyBkColor(COLORREF crColor)
{
    // delete the old brush
    if(m_pBkPropertyBrush) delete m_pBkPropertyBrush;
    
    // create the brush
    m_pBkPropertyBrush = new CBrush(crColor);
    m_crPropertyBkColor = crColor;
}

void CSEPropertyListCtrl::setHighlightColor(COLORREF crColor)
{
    // delete the old brush
    if(m_pBkHighlightBrush) delete m_pBkHighlightBrush;
    
    // create the brush
    m_pBkHighlightBrush = new CBrush(crColor);
    m_crHighlightColor = crColor;
}

bool CSEPropertyListCtrl::addProperty(CUIObjProperty &prop, int nAlignment)
{
    SEPLItem        item;

    item.prop = prop;
    item.nAlignment = nAlignment;
    item.nWidth = getTextExtent(prop.name.c_str());        
    if (prop.valueType == CUIObjProperty::VT_COLOR)
        item.pBrush = new CBrush(getColorValue(prop.strValue.c_str(), RGB(0, 0, 0)));
    else if (prop.valueType == CUIObjProperty::VT_BOOL_STR)
    {
        item.prop.options.push_back(SZ_TRUE);
        item.prop.options.push_back(SZ_FALSE);
    }
    else if (prop.valueType == CUIObjProperty::VT_FILE)
    {
        item.prop.options.clear();
        item.prop.options.push_back("");
        for (int i = 0; i < prop.options.size(); i++)
        {
            g_skinFactory.getResourceMgr()->enumFiles(prop.options[i].c_str(), 
                item.prop.options, false);
        }
    }
    else if (prop.valueType == CUIObjProperty::VT_FONT_NAME)
    {
        strcpy_safe(item.LogFont.lfFaceName, CountOf(item.LogFont.lfFaceName), prop.strValue.c_str());
    }

    m_vProperies.push_back(item);

    // add the string to the list box
    int nPos = CListBox::addString(prop.name.c_str());

    // create the Control if Needed
    createControl(prop.valueType);

    return true;
}

bool CSEPropertyListCtrl::addProperties(CUIObjProperties &properties, int nAlignment)
{
    CUIObjProperties::iterator    it, itEnd;

    itEnd = properties.end();
    for (it = properties.begin(); it != itEnd; ++it)
        addProperty(*it, nAlignment);

    return true;
}

bool CSEPropertyListCtrl::setProperty(cstr_t szProp, cstr_t szValue)
{
    V_SEPLITEM::iterator    it, itEnd;

    itEnd = m_vProperies.end();
    for (it = m_vProperies.begin(); it != itEnd; ++it)
    {
        SEPLItem    &item = *it;
        if (strcasecmp(szProp, item.prop.name.c_str()) == 0)
        {
            item.prop.strValue = szValue;
            assert(item.prop.valueType != CUIObjProperty::VT_IMAGE);
            invalidate();
            return true;
        }
    }

    return false;
}

CUIObjProperty *CSEPropertyListCtrl::getProperty(int nIndex)
{
    if (nIndex >= 0 && nIndex < m_vProperies.size())
    {
        return &(m_vProperies[nIndex].prop);
    }

    return nullptr;
}

void CSEPropertyListCtrl::getProperties(vector<string> &properties)
{
    V_SEPLITEM::iterator    it, itEnd;

    itEnd = m_vProperies.end();
    for (it = m_vProperies.begin(); it != itEnd; ++it)
    {
        SEPLItem    &item = *it;
        item.prop.toUIObjProperties(properties);
    }
}

void CSEPropertyListCtrl::resetContent()
{
    CListBox::resetContent();
    hideControls();
    m_vProperies.clear();
    m_pCurItem = nullptr;
    m_pCurDrawItem = nullptr;
}

void CSEPropertyListCtrl::drawItem(CDC* pDC, CRect itemRect, bool bSelected)
{
    /////////////////////////////////////////
    // paint the Background rectangle (Property Value)
    if (m_pCurDrawItem->prop.valueType == CUIObjProperty::VT_COLOR)
        pDC->SelectObject(m_pCurDrawItem->pBrush);
    else
        pDC->SelectObject(m_pBkBrush);
    pDC->SelectObject(m_pBorderPen);

    // draw the rectangle
    itemRect.left = m_nWidestItem - 1;
    itemRect.top--;
    itemRect.right++;
    pDC->rectangle(itemRect);
    CRect OrginalRect = itemRect;
    
    /////////////////////////////////////////
    // draw the Property Text
    pDC->SetBkMode(TRANSPARENT);
    pDC->SelectObject(m_pBkBrush);
    pDC->SelectObject(m_pTextFont);
    pDC->setTextColor(m_crTextColor);
    drawPropertyText(pDC, itemRect);

    /////////////////////////////////////////
    // paint the Background rectangle (Property Name)
    if( bSelected )
        pDC->SelectObject(m_pBkHighlightBrush);

    // draw the rectangle
    itemRect.right = m_nWidestItem;
    itemRect.left = -1;
    pDC->rectangle(itemRect);
    
    /////////////////////////////////////////
    // paint the Property name Text
    // Is this item selected?
    if( bSelected )
    {        
        if(m_bBoldSelection) pDC->SelectObject(m_pSelectedFont);
        pDC->setTextColor(m_crTextHighlightColor);
        m_pCurItem = m_pCurDrawItem;
        m_CurRect = OrginalRect;
    }
    
    // draw the Text
    itemRect.left += 6;
    itemRect.right -= 5;
    pDC->drawText( m_csText, m_csText.getLength(), itemRect, DT_SINGLELINE|DT_VCENTER|m_pCurDrawItem->nAlignment);
}

void CSEPropertyListCtrl::drawPropertyText(CDC* pDC, CRect itemRect)
{
    itemRect.left += 5;
    switch (m_pCurDrawItem->prop.valueType)
    {
    case CUIObjProperty::VT_INT:
    case CUIObjProperty::VT_BOOL_STR:
    case CUIObjProperty::VT_STR:
    case CUIObjProperty::VT_FILE:
    case CUIObjProperty::VT_VAR_INT:
    case CUIObjProperty::VT_RECT:
    case CUIObjProperty::VT_COMB_STR:
    case CUIObjProperty::VT_FONT_NAME:
    case CUIObjProperty::VT_IMAGE:
    case CUIObjProperty::VT_IMG_FILE:
    case CUIObjProperty::VT_ID:
        pDC->drawText(m_pCurDrawItem->prop.strValue.c_str(), itemRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
        break;
    case CUIObjProperty::VT_COLOR:
        break;
    default:
        assert(0);
        break;
    }
}

void CSEPropertyListCtrl::createControl(CUIObjProperty::VALUE_TYPE valueType)
{
    switch (valueType)
    {
    case CUIObjProperty::VT_INT:
    case CUIObjProperty::VT_STR:
    case CUIObjProperty::VT_VAR_INT:
    case CUIObjProperty::VT_RECT:
        // Edit Window
        if(!m_pEditWnd)
        {
            m_pEditWnd = new CEdit();
            m_pEditWnd->create(WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, CRect(0,0,100,100), this, IDC_P_EDIT);
            m_pEditWnd->setFont(m_pTextFont);
        }
        break;
    case CUIObjProperty::VT_FONT_NAME:
    case CUIObjProperty::VT_IMAGE:
    case CUIObjProperty::VT_IMG_FILE:
    case CUIObjProperty::VT_ID:
        if(!m_pButton)
        {
            m_pButton = new CButton();
            m_pButton->create("...", WS_CHILD|BS_PUSHBUTTON, CRect(0,0,100,100), this, IDC_P_BUTTON);
            m_pButton->setFont(m_pTextFont);
        }
        break;
    case CUIObjProperty::VT_BOOL_STR:
    case CUIObjProperty::VT_COMB_STR:
    case CUIObjProperty::VT_FILE:
        if(!m_pEditWnd)
        {
            m_pEditWnd = new CEdit();
            m_pEditWnd->create(WS_CHILD|ES_AUTOHSCROLL|ES_LEFT, CRect(0,0,100,100), this, IDC_P_EDIT);
            m_pEditWnd->setFont(m_pTextFont);
        }
        if(!m_pListBox)
        {
            m_pListBox = new CListBox();
            m_pListBox->create(WS_CHILD|WS_BORDER|LBS_NOTIFY|WS_VSCROLL|LBS_HASSTRINGS, CRect(0,0,100,100), this, IDC_P_COMB_LIST);
            m_pListBox->setFont(m_pTextFont);
            
            m_pComboButton = new CComboButton();
            m_pComboButton->create(CRect(0,0,0,0), this, IDC_P_COMB_BT ); 
        }
        break;
    case CUIObjProperty::VT_COLOR:
        break;
    default:
        assert(0);
        break;
    }
}

void CSEPropertyListCtrl::hideControls()
{
    // Hide the controls
    if(m_pEditWnd)        m_pEditWnd->showWindow(SW_HIDE);
    if(m_pButton)        m_pButton->showWindow(SW_HIDE);
    if(m_pListBox)        m_pListBox->showWindow(SW_HIDE);
    if(m_pComboButton)    m_pComboButton->showWindow(SW_HIDE);    
}

// Calculate the width of the string based on the font set
int CSEPropertyListCtrl::getTextExtent(cstr_t szText)
{
    CDC* pDC = GetDC();
    if (!pDC)
        return 10;

    pDC->SelectObject(m_pSelectedFont);
    cSize size = pDC->getTextExtent(szText);
    if(size.cx + 10 > m_nWidestItem)
        m_nWidestItem = size.cx + 10;

    ReleaseDC(pDC);

    return size.cx;
}

void CSEPropertyListCtrl::onPropertyChanged(int nIndex)
{
    g_seSkinPropertyAdapter.updatePropertyToSkin();
}

