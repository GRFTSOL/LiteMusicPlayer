//
//  SkinMenuItemsContainer.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/12.
//

#ifndef SkinMenuItemsContainer_hpp
#define SkinMenuItemsContainer_hpp

#include "SkinLinearContainer.h"
#include "SkinButton.h"


class SkinMenuItemsContainer : public CSkinContainer {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    SkinMenuItemsContainer();
    virtual ~SkinMenuItemsContainer();

    void onCreate() override;
    int fromXML(SXNode *pXmlNode) override;
    void onSize() override;

protected:
    int getItemsMinSumHeight();

protected:
    CSkinButton             *m_btnUp, *m_btnDown;
    CSkinLinearContainer    *m_linearContainer;
    bool                    m_isArrowButtonAdded;

};

#endif /* SkinMenuItemsContainer_hpp */
