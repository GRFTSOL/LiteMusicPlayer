#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include "GfxRaw.h"
#include "RawGraphData.h"
#include "../../Window/mac/WindowHandleHolder.h"


CGContextRef createViewContext(ViewMacImp *view) {
    CGContextRef viewContext = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    assert(viewContext);

    CGContextSaveGState(viewContext);

    CGContextTranslateCTM(viewContext, 0, [view frame].size.height);
    CGContextScaleCTM(viewContext, 1.0, -1.0);

    return viewContext;
}

CRawGraphData::CRawGraphData() {
    m_context = nullptr;
    m_windowHandle = nullptr;
}

CRawGraphData::~CRawGraphData() {
    destroy();
}

bool CRawGraphData::create(int cx, int cy, WindowHandleHolder *windowHandle, int nBitCount) {
    assert(cx > 0 && cy > 0);
    assert(nBitCount == 32);
    m_windowHandle = windowHandle;

    if (!m_imageData.create(cx, cy, nBitCount)) {
        return false;
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    // create the bitmap context
    m_context = CGBitmapContextCreate(m_imageData.buff, m_imageData.width, m_imageData.height, 8,
        m_imageData.absStride(), colorSpace,
        // this will give us an optimal RGBA format for the device:
        (kCGImageAlphaPremultipliedLast));
    assert(m_context);
    CGColorSpaceRelease(colorSpace);

    return m_context != nullptr;
}

void CRawGraphData::destroy() {
    if (m_context != nullptr) {
        CGContextRelease(m_context);
    }

    m_imageData.free();
}

void CRawGraphData::drawToWindow(int xdest, int ydest, int width, int height, int xsrc, int ysrc, float scaleFactor) {
    assert(m_context);

    CGImageRef rawImage = CGBitmapContextCreateImage(m_context);
    CGRect rcDest = CGRectMake(xdest, ydest, width, height);

    if (xsrc + width >= m_imageData.width) {
        width = m_imageData.width - xsrc;
    }
    if (ysrc + height >= m_imageData.height) {
        height = m_imageData.height - ysrc;
    }

    CGContextRef viewContext = createViewContext(m_windowHandle->view);

    if (xsrc == 0 && ysrc == 0 && m_imageData.width == width && m_imageData.height == height) {
        CGContextDrawImage(viewContext, rcDest, rawImage);
        CGContextRestoreGState(viewContext);
        CGImageRelease(rawImage);
        return;
    }

    CGRect rcSrc = CGRectMake(xsrc * scaleFactor, m_imageData.height - (height + ydest) * scaleFactor,
        width * scaleFactor, height * scaleFactor);
    CGImageRef partImage = CGImageCreateWithImageInRect(rawImage, rcSrc);

    CGContextDrawImage(viewContext, rcDest, partImage);

    CGContextRestoreGState(viewContext);
    CGImageRelease(partImage);
    CGImageRelease(rawImage);
}
