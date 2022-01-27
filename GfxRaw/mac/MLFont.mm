#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include "FontInfo.h"
#include "MLFont.h"

CMLFont::CMLFont()
{
    m_font = nullptr;
}

CMLFont::~CMLFont(void)
{
    destroy();
}

CTFontDescriptorRef CreateFontDescriptorFromFamilyAndTraits(CFStringRef iFamilyName,
                                                            CTFontSymbolicTraits iTraits, CGFloat iSize)
{
    CTFontDescriptorRef descriptor = nullptr;
    CFMutableDictionaryRef attributes;
    
    assert(iFamilyName != nullptr);
    // create a mutable dictionary to hold our attributes.
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if (attributes != nullptr) {
        CFMutableDictionaryRef traits;
        CFNumberRef symTraits;
        
        // Add a family name to our attributes.
        CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, iFamilyName);
        
        // create the traits dictionary.
        symTraits = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type,
                                   &iTraits);
        
        if (symTraits != nullptr) {
            // create a dictionary to hold our traits values.
            traits = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                               &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            
            if (traits != nullptr) {
                // Add the symbolic traits value to the traits dictionary.
                CFDictionaryAddValue(traits, kCTFontSymbolicTrait, symTraits);
                
                // Add the traits attribute to our attributes.
                CFDictionaryAddValue(attributes, kCTFontTraitsAttribute, traits);
                CFRelease(traits);
            }
            CFRelease(symTraits);
        }
        // create the font descriptor with our attributes and input size.
        descriptor = CTFontDescriptorCreateWithAttributes(attributes);
        
        CFRelease(attributes);
    }
    // Return our font descriptor.
    return descriptor;
}

CTFontRef CreateFont(CTFontDescriptorRef iFontDescriptor, CGFloat iSize)
{
    // create the font from the font descriptor and input size. Pass
    // nullptr for the matrix parameter to use the default matrix (identity).
    
    return CTFontCreateWithFontDescriptor(iFontDescriptor, iSize, nullptr);
}

CGFloat GetLineHeightForFont(CTFontRef iFont)
{
    CGFloat lineHeight = 0.0;
    
    // Get the ascent from the font, already scaled for the font's size
    lineHeight += CTFontGetAscent(iFont);
    
    // Get the descent from the font, already scaled for the font's size
    lineHeight += CTFontGetDescent(iFont);
    
    // Get the leading from the font, already scaled for the font's size
    lineHeight += CTFontGetLeading(iFont);
    
    return lineHeight;
}

bool CMLFont::create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nSize, int nWeight, int nItalic, bool bUnderline)
{
    string    strFontName;

    CFontInfo::create(szFaceNameLatin9, szFaceNameOthers, nSize, nWeight, nItalic, bUnderline);

    if (szFaceNameLatin9 == nullptr || isEmptyString(szFaceNameLatin9))
        szFaceNameLatin9 = "Helvetica";

    for (int i = 0; i < 2; i++) {
        CFStringRef    name = CFStringCreateWithCString(nullptr, szFaceNameLatin9, kCFStringEncodingUTF8);
        CTFontSymbolicTraits fontTraits = 0;
        if (nWeight == FW_BOLD)
            fontTraits |= kCTFontBoldTrait;
        if (nItalic)
            fontTraits |= kCTFontItalicTrait;
        m_font = CreateFont(
                            CreateFontDescriptorFromFamilyAndTraits(name, fontTraits, nSize),
                            nSize);
        CFRelease(name);
        if (m_font != nullptr)
            break;
        szFaceNameLatin9 = "Helvetica";
    }
    
    if (m_font == nullptr)
        return false;

    CGFloat lineHeight = GetLineHeightForFont(m_font);
    m_nFontHeight = (int)(lineHeight + 0.99);

    return true;
}

bool CMLFont::create(const CFontInfo &font)
{
    return create(font.getName(), font.getNameOthers(), font.getSize(), font.getWeight(), font.getItalic(), font.isUnderline());
}

void CMLFont::destroy()
{
    if (m_font != nullptr)
    {
        CFRelease(m_font);
        m_font = nullptr;
    }
}
