// BmpFile.cpp: implementation of the CBmpFile class.
//
//////////////////////////////////////////////////////////////////////

#include "ILIO.h"
#include "RawImageData.h"

#ifndef _WIN32

#pragma pack(push)
#pragma pack(1)
typedef long LONG;

typedef struct tagBITMAPFILEHEADER {
    uint16_t    bfType;
    uint32_t   bfSize;
    uint16_t    bfReserved1;
    uint16_t    bfReserved2;
    uint32_t   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
    uint32_t      biSize;
    int32_t      biWidth;
    int32_t      biHeight;
    uint16_t       biPlanes;
    uint16_t       biBitCount;
    uint32_t      biCompression;
    uint32_t      biSizeImage;
    int32_t      biXPelsPerMeter;
    int32_t      biYPelsPerMeter;
    uint32_t      biClrUsed;
    uint32_t      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

#pragma pack(pop)

#endif

RawImageData *loadRawImageDataFromBmpFile(IILIO *io)
{
    RawImageData *pImage = nullptr;
    BITMAPFILEHEADER    bfh;
    BITMAPINFOHEADER    bih;
    uint32_t                dwDataSize;
    size_t                nFileLen;
    int                    nClrUsed = 0;

    io->getSize(nFileLen);

    if (nFileLen <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
        goto FAILED;

    if (io->read(&bfh, sizeof(bfh)) != sizeof(bfh))
        goto FAILED;
    if (bfh.bfType != MAKEWORD((uint8_t)'B',(uint8_t)'M'))
        goto FAILED;

    if (io->read(&bih, sizeof(bih)) != sizeof(bih))
        goto FAILED;

    switch (bih.biBitCount)
    {
    case 8:                                /*Colors counts of RGBQUAD */
    case 4:
    case 2:
    case 1:
        nClrUsed = (bih.biClrUsed > 0 ? bih.biClrUsed : (2 << (bih.biBitCount - 1)));
        break;
    case 32:
    case 24:
    case 16:
    default:
        nClrUsed = 0;
        break;
    }

    // create raw image
    pImage = new RawImageData;
    assert(pImage);
    if (!pImage->create(bih.biWidth, bih.biHeight, bih.biBitCount))
        goto FAILED;

    if (nClrUsed > 0)
    {
        pImage->setClrUsed(nClrUsed);
        if (io->read(pImage->pallete, sizeof(RGBQUAD) * nClrUsed) != sizeof(RGBQUAD) * nClrUsed)
            goto FAILED;
    }

    dwDataSize = pImage->getBuffSize();
    if (io->read(pImage->buff, dwDataSize) != dwDataSize)
        goto FAILED;

    return pImage;
FAILED:
    if (pImage)
    {
        if (pImage->buff)
            pImage->free();

        delete pImage;
    }

    return nullptr;
}

int saveBmpFileFromRawImageData(RawImageData *pImageData, cstr_t szFile)
{
    BITMAPFILEHEADER    bmfh;
    FILE                *fp;

    fp = fopen(szFile, "wb");
    if (!fp)
    {
        ERR_LOG1("Can't create file: %s", szFile);
        return ERR_OPEN_FILE;
    }

    bmfh.bfSize = sizeof(bmfh);
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfType = MAKEWORD((uint8_t)'B',(uint8_t)'M');

    // write BITMAPFILEHEADER
    if (fwrite(&bmfh, 1, sizeof(bmfh), fp) != sizeof(bmfh))
        goto FAILED;

    BITMAPINFO    bmpInfo;
    memset(&bmpInfo, 0, sizeof(bmpInfo));
    bmpInfo.bmiHeader.biBitCount = pImageData->bitCount;
    bmpInfo.bmiHeader.biHeight = pImageData->height;
    bmpInfo.bmiHeader.biWidth = pImageData->width;
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biSizeImage = pImageData->getBuffSize();

    // write BITMAPINFO
    if (fwrite(&bmpInfo, 1, bmpInfo.bmiHeader.biSize, fp) != bmpInfo.bmiHeader.biSize)
        goto FAILED;

    // write Data
    if (fwrite(pImageData->buff, 1, pImageData->getBuffSize(), fp) != pImageData->getBuffSize())
        goto FAILED;

    fclose(fp);

    return ERR_OK;
FAILED:
    fclose(fp);

    return ERR_WRITE_FILE;
}
