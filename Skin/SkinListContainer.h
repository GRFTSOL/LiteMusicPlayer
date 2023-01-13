//
//  SkinListContainer.h
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/12.
//

#ifndef SkinListContainer_h
#define SkinListContainer_h

#include "SkinContainer.h"


/**
 * CSkinListContainer 包含了多个列表
 * - 可设置点击到外表就关闭
 * - 按 Escape 关闭
 * - 丢失 focus 关闭等
 * - 可弹出多层 CSkinPopContainer
 */
class CSkinListContainer : public CSkinContainer {
public:
    CSkinListContainer();
    virtual ~CSkinListContainer();

};

#endif /* SkinListContainer_h */
