#pragma once

#ifndef _SKIN_IMAGE_INC_
#define _SKIN_IMAGE_INC_

#include "../GfxRaw/RawImage.h"


class CSkinWnd;
class CSkinResMgr;

// CSFImage = CSkinFactoryImage
class CSFImage : public CRawImage {
public:
    CSFImage(void);
    CSFImage(const CSFImage &src);
    virtual ~CSFImage(void);

    CSFImage &operator=(const CSFImage &src);

    bool loadFromSRM(CSkinWnd *skinWnd, cstr_t resName);

    virtual void detach() override;

    virtual const RawImageDataPtr &getRawImageData(float scaleFactor) override;

    virtual bool isPixelTransparent(CPoint pt) const override;

protected:
    void copyFrom(const CSFImage &src);

protected:
    CSkinResMgr                 *m_skinResMgr;
    string                      m_resName;
    float                       m_scaleFactor;

};

#endif // _SKIN_IMAGE_INC_
