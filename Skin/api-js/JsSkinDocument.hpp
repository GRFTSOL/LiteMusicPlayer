//
//  JsSkinDocument.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/22.
//

#pragma once

#ifndef JsSkinDocument_hpp
#define JsSkinDocument_hpp

#include "objects/JsObjectX.hpp"


class CSkinWnd;

class JsSkinDocument : public JsObjectX {
public:
    JsSkinDocument(CSkinWnd *skinWnd);

    virtual bool onSetValue(VMContext *ctx, const SizedString &name, const JsValue &value) override;
    virtual JsValue onGetValue(VMContext *ctx, const SizedString &name) override;
    virtual void onEnumAllProperties(VMContext *ctx, VecSizedStrings &names, VecJsValues &values) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    CSkinWnd *skinWnd() const { return _skinWnd; }

protected:
    CSkinWnd                    *_skinWnd;

};

#endif /* JsSkinDocument_hpp */
