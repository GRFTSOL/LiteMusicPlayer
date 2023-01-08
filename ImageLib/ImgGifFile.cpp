// ImgGifFile.cpp: implementation of the CImgGifFile class.
//
//////////////////////////////////////////////////////////////////////

#include "RawImageData.h"
#include "ILIO.h"
#include "ImgGifFile.h"

void setPalette(RGBQUAD *pal, int n, uint8_t *r, uint8_t *g, uint8_t *b);

class GifRawImageData : public RawImageData
{
public:
    GifRawImageData() {
        m_nTransIndex = -1;
        m_nOffsetX = 0;
        m_nOffsetY = 0;
    }

    void copy(GifRawImageData *src) {
        size_t nBufSize = src->getBuffSize();
        buff = new uint8_t[nBufSize];
        width = src->width;
        height = src->height;
        stride = src->stride;
        pixFormat = src->pixFormat;
        setClrUsed(src->nClrUsed);

        memcpy(buff, src->buff, nBufSize);
        if (src->nClrUsed)
            memcpy(pallete, src->pallete, sizeof(RGBQUAD) * 256);

        m_nTransIndex = src->m_nTransIndex;
    }

    int getTransIndex() { return m_nTransIndex; }
    void setTransIndex(int nTransIndex) { m_nTransIndex = nTransIndex; }
    bool isTransparent() const { return m_nTransIndex >= 0; }

    RGBQUAD getTransColor()
    {
        if (bitCount <= 8 && m_nTransIndex != -1)
            return pallete[(uint8_t)m_nTransIndex];

        return m_clrTrans;
    }

    void setTransColor(RGBQUAD rgb) { m_clrTrans = rgb; }

    uint8_t getPixelIndex(int x, int y)
    {
        if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
        {
            return 0;
        }
        if (pixFormat == PF_PALLETE256)
            return rowPtr(y)[x];
        else
        {
            uint8_t pos;
            uint8_t iDst;
            uint8_t *row;
            row = rowPtr(y);
            if (pixFormat == PF_PALLETE16)
            {
                iDst = row[(x * 4) >> 3];
                pos = (uint8_t)(4 * (1 - x % 2));
                iDst &= (0x0F << pos);
                return (uint8_t)(iDst >> pos);
            }
            else
            {
                assert(pixFormat == PF_PALLETE2);
                iDst = row[(x * 1) >> 3];
                pos = (uint8_t)(7-x%8);
                iDst &= (0x01<<pos);
                return (uint8_t)(iDst >> pos);
            }
        }
        return 0;
    }

    void setPixelColor(int x, int y, const RGBQUAD &c)
    {
        if ((x < 0) || (y < 0) || 
            (x >= width) || (y >= height))
            return;

        if (nClrUsed)
            setPixelIndex(x, y, getNearestIndex(c));
        else
        {
            uint8_t* iDst = rowPtr(y) + x * 3;
            *iDst++ = c.rgbBlue;
            *iDst++ = c.rgbGreen;
            *iDst   = c.rgbRed;
        }
    }

    RGBQUAD getPixelColor(int x, int y)
    {
        RGBQUAD        c;
        if ((x < 0) || (y < 0) || 
            (x >= width) || (y >= height))
        {
            c.rgbRed = c.rgbGreen = c.rgbBlue = 0;
            c.rgbReserved = 255;
            return c;
        }

        if (nClrUsed)
            c = pallete[getPixelIndex(x, y)];
        else
        {
            uint8_t* iDst = rowPtr(y);
            if (bitCount == 24)
            {
                iDst += x * 3;
                c.rgbReserved = 255;
            }
            else
            {
                iDst += x * 4;
                c.rgbReserved = iDst[3];
            }
            c.rgbBlue  = *iDst++;
            c.rgbGreen = *iDst++;
            c.rgbRed   = *iDst;
        }

        return c;
    }

    void setPaletteIndex(uint8_t n, RGBQUAD &c)
    {
        assert(bitCount <= 8);

        pallete[n] = c;
    }

    RGBQUAD getPaletteColor(uint8_t idx) { return pallete[idx]; }

    void setPixelIndex(int x, int y, uint8_t i)
    {
        if (nClrUsed == 0 ||
            (x < 0) || (y < 0) || (x >= width) || (y >= height))
            return ;

        if (pixFormat == PF_PALLETE256)
        {
            rowPtr(y)[x] = i;
            return;
        }
        else
        {
            uint8_t pos;
            uint8_t* iDst;
            iDst = rowPtr(y);
            if (pixFormat == PF_PALLETE16)
            {
                iDst += (x * 4) >> 3;
                pos = (uint8_t)(4*(1-x%2));
                *iDst &= ~(0x0F<<pos);
                *iDst |= ((i & 0x0F)<<pos);
                return;
            }
            else if (pixFormat == PF_PALLETE2)
            {
                iDst += (x * 1) >> 3;
                pos = (uint8_t)(7-x%8);
                *iDst &= ~(0x01<<pos);
                *iDst |= ((i & 0x01)<<pos);
                return;
            }
        }
    }

    uint8_t getNearestIndex(RGBQUAD c)
    {
        if (nClrUsed == 0)
            return 0;

        RGBQUAD* ppal= pallete;
        int            nPalette;

        nPalette = nClrUsed;

        int    j = 0;
        long distance=200000;
        long k;
        for (int i = 0; i < nPalette; i++)
        {
            k = (ppal[i].rgbBlue - c.rgbBlue) * (ppal[i].rgbBlue - c.rgbBlue)+
                (ppal[i].rgbGreen - c.rgbGreen) * (ppal[i].rgbGreen - c.rgbGreen)+
                (ppal[i].rgbRed - c.rgbRed) * (ppal[i].rgbRed - c.rgbRed);
    //        k = abs(iDst[l]-c.rgbBlue)+abs(iDst[l+1]-c.rgbGreen)+abs(iDst[l+2]-c.rgbRed);
            if (k == 0)
            {
                j = i;
                break;
            }

            if (k < distance)
            {
                distance = k;
                j=i;
            }
        }

        return (uint8_t)j;
    }

    void setCurFrame(int nCurFrame) { m_nCurFrame = nCurFrame; }

    void setOffset(int x, int y) { m_nOffsetX = x; m_nOffsetY = y; }

    int             m_nCurFrame;
    int             m_nTransIndex;
    RGBQUAD         m_clrTrans;
    uint32_t        m_dwFrameDelay;
    int             m_nOffsetX, m_nOffsetY;

};

void gifMix(GifRawImageData *imgSrc, GifRawImageData *imgTarg, int lxOffset, int lyOffset) {
    int lWide = min((int)imgTarg->width, imgSrc->width - lxOffset);
    int lHeight = min((int)imgTarg->height, imgSrc->height - lyOffset);

    uint8_t ibg2 = (uint8_t)imgSrc->getTransIndex();
    uint8_t i2;

    for (int y = 0; y < lHeight; y++)
    {
        for (int x = 0; x < lWide; x++)
        {
            i2 = imgSrc->getPixelIndex(x + lxOffset, y + lyOffset);
            if (i2 != ibg2)
                imgTarg->setPixelIndex(x, y, i2);
        }
    }
}

bool fwritec(IILIO *io, unsigned char c)
{
    return io->write(&c, 1) == 1;
}

RawImageDataPtr loadRawImageDataFromGifFile(IILIO *io)
{
    CImgGifFile        file;

    file.m_imgData = make_shared<GifRawImageData>();
    if (!file.open(io)) {
        return nullptr;
    }

    file.m_imgData->stride = -file.m_imgData->stride;

    return file.m_imgData;
}

bool CImgGifFile::open(IILIO *io)
{
    /* AD - for transparency */
    struct_dscgif dscgif;
    struct_image image;
    struct_TabCol TabCol;

    if (io->read(&dscgif, sizeof(dscgif)) != sizeof(dscgif))
        return nullptr;
    //if (strncmp(dscgif.header,"GIF8",3)!=0) {
    if (strncmp(dscgif.header,"GIF8",4) != 0)
        return nullptr;

    /* AD - for interlace */
    TabCol.sogct = (short)(1 << ((dscgif.pflds & 0x07)+1));
    TabCol.colres = (short)(((dscgif.pflds & 0x70) >> 3) + 1);

    // assume that the image is a truecolor-gif if
    // 1) no global color map found
    // 2) (image.w, image.h) of the 1st image != (dscgif.scrwidth, dscgif.scrheight)
    long bTrueColor=0;
    GifRawImageDataPtr imaRGB;

    // Global colour map?
    if (dscgif.pflds & 0x80)
    {
        if (io->read(TabCol.paleta, sizeof(struct rgb_color)*TabCol.sogct) != sizeof(struct rgb_color)*TabCol.sogct)
            return nullptr;
    }
    else 
        bTrueColor++;    //first chance for a truecolor gif

    int first_transparent_index;

    int        iImage = 0;
    int        nNumFrames;
    nNumFrames = get_num_frames(io, &TabCol);

    if (nNumFrames <= 0)
        return nullptr;

    char ch;
    for (bool bContinue = true; bContinue; )
    {
        if (io->read(&ch, sizeof(ch)) != sizeof(ch))
            {break;}

        switch (ch)
        {
        case '!': // extension
            {
            bContinue = decodeExtension(io);
            break;
            }
        case ',': // image
            {
            assert(sizeof(image) == 9);
            if (io->read(&image, sizeof(image)) != sizeof(image))
            {
                return false;
            }
            //avoid uint8_t order problems with Solaris <candan>
            uint8_t *byteData = (uint8_t *) & image;
            image.l = byteData[0]+byteData[1]*256;
            image.t = byteData[2]+byteData[3]*256;
            image.w = byteData[4]+byteData[5]*256;
            image.h = byteData[6]+byteData[7]*256;

            // check if it could be a truecolor gif
            if ((iImage==0) && (image.w != dscgif.scrwidth) && (image.h != dscgif.scrheight))
                bTrueColor++;

            // Local colour map?
            if (image.pf & 0x80) {
                TabCol.sogct = (short)(1 << ((image.pf & 0x07) +1));
                assert(3 == sizeof(struct rgb_color));
                if (io->read(TabCol.paleta, sizeof(struct rgb_color)*TabCol.sogct) != sizeof(struct rgb_color)*TabCol.sogct)
                {
                    return false;
                }
                //log << "Local colour map" << endl;
            }

            int bpp; //<DP> select the correct bit per pixel value
            if (TabCol.sogct <= 2)  bpp = 1;
            else if (TabCol.sogct <= 16) bpp = 4;
            else bpp = 8;

            //handle Disposal Method
            GifRawImageDataPtr previmage = make_shared<GifRawImageData>();
            if (iImage>0 && gifgce.dispmeth==1) previmage->copy(m_imgData.get());
            if (iImage==0)    first_transparent_index = m_imgData->m_nTransIndex;

            m_imgData->create(image.w, image.h, bpp);

            if ((image.pf & 0x80) || (dscgif.pflds & 0x80)) {
                unsigned char r[256], g[256], b[256];
                int i;

                for (i=0; i < TabCol.sogct; i++) {
                    r[i] = TabCol.paleta[i].r;
                    g[i] = TabCol.paleta[i].g;
                    b[i] = TabCol.paleta[i].b;

                }

                // Force transparency colour white...
                //if (0) if (info.nBkgndIndex != -1)
                //    r[info.nBkgndIndex] = g[info.nBkgndIndex] = b[info.nBkgndIndex] = 255;
                // Fill in with white // AD
                if (m_imgData->m_nTransIndex != -1) {
                    while (i < 256)    {
                        r[i] = g[i] = b[i] = 255;
                        i++;
                    }
                }

                // Force last colour to white...   // AD
                //if ((info.nBkgndIndex != -1) && !has_white) {
                //    r[255] = g[255] = b[255] = 255;
                //}

                setPalette(m_imgData->pallete, (m_imgData->m_nTransIndex != -1 ? 256 : TabCol.sogct), r, g, b);
            }

            CImageIterator* iter = new CImageIterator(m_imgData);
            iter->upset();
            int badcode;
            ibf = GIFBUFTAM+1;

            interlaced = image.pf & 0x40;
            iheight = image.h;
            istep = 8;
            iypos = 0;
            ipass = 0;

            //if (interlaced) log << "Interlaced" << endl;
            decoder(io, iter, image.w, badcode);
            delete iter;

            //handle Disposal Method
            /*Values :  0 -   No disposal specified. The decoder is
                              not required to take any action.
                        1 -   Do not dispose. The graphic is to be left
                              in place.
                        2 -   Restore to background color. The area used by the
                              graphic must be restored to the background color.
                        3 -   Restore to previous. The decoder is required to
                              restore the area overwritten by the graphic with
                              what was there prior to rendering the graphic.
            */
            if (iImage>0 && gifgce.dispmeth==1 && bTrueColor<2){
                gifMix(m_imgData.get(), previmage.get(), -image.l, -(int)previmage->height + image.t + image.h);
                previmage->setTransIndex(first_transparent_index);
                m_imgData = previmage;
            }

            // restore the correct position in the file for the next image
            io->seek(-(ibfmax - ibf - 1), SEEK_CUR);

            if (bTrueColor >= 2){ //it's a truecolor gif!
                //force full image decoding
                m_imgData->setCurFrame(nNumFrames - 1);
                //build the RGB image
                if (imaRGB == nullptr)
                {
                    imaRGB = make_shared<GifRawImageData>();
                    imaRGB->create(dscgif.scrwidth, dscgif.scrheight, 24);
                }
                //copy the partial image into the full RGB image
                for (int y = 0; y < image.h; y++)
                {
                    for (int x = 0; x < image.w; x++)
                    {
                        imaRGB->setPixelColor(x + image.l, dscgif.scrheight - 1 - image.t - y,
                            m_imgData->getPixelColor(x, image.h - y - 1));
                    }
                }
            }

            if (m_imgData->m_nCurFrame == iImage)
                bContinue=false;
            else
                iImage++;
            break;
            }
        case ';': //terminator
            bContinue=false;
            break;
        default:
            break;
        }
   }

    if (bTrueColor >= 2 && imaRGB){
        if (gifgce.transpcolflag){
            imaRGB->setTransColor(m_imgData->getTransColor());
            imaRGB->setTransIndex(0);
        }
        m_imgData = imaRGB;
    }

    return true;
}

bool CImgGifFile::decodeExtension(IILIO *io)
{
    bool bContinue;
    unsigned char count;
    unsigned char fc;

    bContinue = (sizeof(fc) == io->read(&fc, sizeof(fc)));
    if (bContinue) {
        /* AD - for transparency */
        if (fc == 0xF9)    {
            bContinue = (sizeof(count) == io->read(&count, sizeof(count)));
            if (bContinue) {
                assert(sizeof(gifgce) == 4);
                bContinue = (sizeof(gifgce) == io->read(&gifgce, sizeof(gifgce)));
                if (bContinue) {
                    if (gifgce.transpcolflag)
                        m_imgData->setTransIndex(gifgce.transpcolindex);
                    m_imgData->m_dwFrameDelay = gifgce.delaytime;
                    m_dispmeth = gifgce.dispmeth;
        }    }    }

        if (fc == 0xFE) { //<DP> Comment block
            bContinue = (sizeof(count) == io->read(&count, sizeof(count)));
            if (bContinue) {
                bContinue = (count == io->read(m_comment, count));
                m_comment[count]='\0';
        }    }

        if (fc == 0xFF) { //<DP> Application Extension block
            bContinue = (sizeof(count) == io->read(&count, sizeof(count)));
            if (bContinue) {
                bContinue = (count==11);
                if (bContinue){
                    char AppID[11];
                    bContinue = (count == io->read(AppID, count));
                    if (bContinue) {
                        bContinue = (sizeof(count) == io->read(&count, sizeof(count)));
                        if (bContinue) {
                            bContinue = (count==3);
                            if (bContinue){
                                uint8_t dati[3];
                                bContinue = (count == io->read(dati, count));
                                if (bContinue){
                                    m_loops = dati[1]+256*dati[2];
        }    }    }    }    }    }    }

        while (bContinue && io->read(&count, sizeof(count)) && count) {
            //log << "Skipping " << count << " bytes" << endl;
            io->seek(count, SEEK_CUR);
        }
    }
    return bContinue;
}


//   - This external (machine specific) function is expected to return
// either the next uint8_t from the GIF file, or a negative error number.
int CImgGifFile::get_byte(IILIO* io)
{
    if (ibf >= GIFBUFTAM)
    {
        // FW 06/02/98 >>>
        ibfmax = (int)io->read(buf, GIFBUFTAM);
        // ibfmax = file->read( buf , 1 , GIFBUFTAM) ;
        if( ibfmax < GIFBUFTAM ) buf[ ibfmax ] = 255 ;
        // FW 06/02/98 <<<
        ibf = 0;
    }
    if (ibf>=ibfmax) return -1; //<DP> avoid overflows
    return buf[ibf++];
}
////////////////////////////////////////////////////////////////////////////////
/*   - This function takes a full line of pixels (one uint8_t per pixel) and
 * displays them (or does whatever your program wants with them...).  It
 * should return zero, or negative if an error or some other event occurs
 * which would require aborting the decode process...  Note that the length
 * passed will almost always be equal to the line length passed to the
 * decoder function, with the sole exception occurring when an ending code
 * occurs in an odd place in the GIF file...  In any case, linelen will be
 * equal to the number of pixels passed...
*/
int CImgGifFile::out_line(CImageIterator *iter, unsigned char *pixels, int linelen)
{
    //<DP> for 1 & 4 bpp images, the pixels are compressed
    if (m_imgData->bitCount < 8)
    {
        for(long x=0;x<m_imgData->width;x++){
            uint8_t pos;
            uint8_t* iDst= pixels + (x*m_imgData->bitCount >> 3);
            if (m_imgData->bitCount == 4){
                pos = (uint8_t)(4*(1-x%2));
                *iDst &= ~(0x0F<<pos);
                *iDst |= ((pixels[x] & 0x0F)<<pos);
            } else if (m_imgData->bitCount == 1){
                pos = (uint8_t)(7-x%8);
                *iDst &= ~(0x01<<pos);
                *iDst |= ((pixels[x] & 0x01)<<pos);
            }
        }
    }

    /* AD - for interlace */
    if (interlaced) {
        iter->setY(iheight-iypos-1);
        iter->setRow(pixels, linelen);

        if ((iypos += istep) >= iheight) {
            do {
                if (ipass++ > 0) istep /= 2;
                iypos = istep / 2;
            }
            while (iypos > iheight);
        }
        return 0;
    } else {
        if (iter->itOK()) {
            iter->setRow(pixels, linelen);
            (void)iter->prevRow();
            return 0;
        } else {
            //     puts("chafeo");
            return -1;
        }
    }
}
// 
// bool CImgGifFile::save(cstr_t szFile)
// {
//     FILE        *fp;
// 
//     fp = _tfopen(szFile, "wb");
//     if (!fp)
//         return false;
// 
//     if (m_imgData->GetBitCount() > 8)    {
//         //strcpy(info.szLastError,"GIF Images must be 8 bit or less");
//         //return false;
//         return encodeRGB(fp);
//     }
// 
//     encodeHeader(fp);
// 
//     encodeExtension(fp);
// 
//     encodeBody(fp);
// 
//     encodeComment(fp);
// 
//     fwritec(fp, ';'); // write the GIF file terminator
// 
//     fclose(fp);
// 
//     return true; // done!
// }
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::encodeHeader(IILIO *io)
{
    io->write("GIF89a", 6);       //GIF Header

    Putword(m_imgData->width, io);               //Logical screen descriptor
    Putword(m_imgData->height, io);

    uint8_t Flags;
    if (m_imgData->nClrUsed == 0)
    {
        Flags=0x11;
    }
    else
    {
        Flags = 0x80;
        Flags |=(m_imgData->bitCount - 1) << 5;
        Flags |=(m_imgData->bitCount - 1);
    }

    fwritec(io, Flags); //GIF "packed fields"
    fwritec(io, 0);     //GIF "BackGround"
    fwritec(io, 0);     //GIF "pixel aspect ratio"

    if (m_imgData->nClrUsed != 0)
    {
        RGBQUAD* pPal = m_imgData->pallete;
        int        i;
        for (i=0; i < m_imgData->nClrUsed; ++i) 
        {
            fwritec(io, pPal[i].rgbRed);
            fwritec(io, pPal[i].rgbGreen);
            fwritec(io, pPal[i].rgbBlue);
        }
        for (; i < 256; i++)
        {
            fwritec(io, 0);
            fwritec(io, 0);
            fwritec(io, 0);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::encodeExtension(IILIO *io)
{
    // TRK BEGIN : transparency
    fwritec(io, '!');
    fwritec(io, TRANSPARENCY_CODE);

    gifgce.transpcolflag = (m_imgData->getTransIndex() != -1) ? 1 : 0;
    gifgce.userinputflag = 0;
    gifgce.dispmeth = m_dispmeth;
    gifgce.res = 0;
    gifgce.delaytime = (uint16_t)m_imgData->m_dwFrameDelay;
    gifgce.transpcolindex = (uint8_t)m_imgData->getTransIndex();
    fwritec(io, sizeof(gifgce));
    io->write(&gifgce, sizeof(gifgce));
    fwritec(io, 0);
    // TRK END
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::encodeLoopExtension(IILIO *io)
{
    fwritec(io, '!');        //uint8_t  1  : 33 (hex 0x21) GIF Extension code
    fwritec(io, 255);        //uint8_t  2  : 255 (hex 0xFF) Application Extension Label
    fwritec(io, 11);        //uint8_t  3  : 11 (hex (0x0B) Length of Application Block (eleven bytes of data to follow)
    io->write("NETSCAPE2.0",11);
    fwritec(io, 3);            //uint8_t 15  : 3 (hex 0x03) Length of Data Sub-Block (three bytes of data to follow)
    fwritec(io, 1);            //uint8_t 16  : 1 (hex 0x01)
    Putword(m_loops,io); //bytes 17 to 18 : 0 to 65535, an unsigned integer in lo-hi uint8_t format. 
                        //This indicate the number of iterations the loop should be executed.
    fwritec(io, 0);            //bytes 19       : 0 (hex 0x00) a Data Sub-block Terminator. 
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::encodeBody(IILIO *io, bool bLocalColorMap)
{
    curx = 0;
    cury = m_imgData->height - 1;    //because we read the image bottom to top
    CountDown = (long)m_imgData->width * (long)m_imgData->height;

    fwritec(io, ',');

    Putword(m_imgData->m_nOffsetX, io);
    Putword(m_imgData->m_nOffsetX, io);
    Putword(m_imgData->width,io);
    Putword(m_imgData->height,io);

    uint8_t Flags=0x00; //non-interlaced (0x40 = interlaced) (0x80 = LocalColorMap)
    if (bLocalColorMap)    { Flags|=0x80; Flags|=m_imgData->bitCount-1; }
    fwritec(io, Flags);

    if (bLocalColorMap){
        RGBQUAD* pPal = m_imgData->pallete;
        for(uint32_t i=0; i<m_imgData->nClrUsed; ++i) 
        {
            fwritec(io, pPal[i].rgbRed);
            fwritec(io, pPal[i].rgbGreen);
            fwritec(io, pPal[i].rgbBlue);
        }
    }

    int InitCodeSize = m_imgData->bitCount <=1 ? 2 : m_imgData->bitCount;
     // write out the initial code size
    fwritec(io, (uint8_t)InitCodeSize);

     // Go and actually compress the data
    compressLZW(InitCodeSize+1, io);
/*
    switch (info.dwEncodeOption)
    {
    case 1:    //uncompressed
        compressNONE(InitCodeSize+1, io);
        break;
    case 2: //LZW
        break;
    default: //RLE
        compressRLE(InitCodeSize+1, io);
    }
*/

     // write out a Zero-length packet (to end the series)
    fwritec(io, 0);
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::encodeComment(IILIO *io)
{
    long n=strlen(m_comment);
    if (n>255) n=255;
    if (n) {
        fwritec(io, '!');    //extension code:
        fwritec(io, 254);    //comment extension
        fwritec(io, (uint8_t)n);    //size of comment
        io->write(m_comment,n);
        fwritec(io, 0);    //block terminator
    }
}

//////////////////////////////////////////////////////////////////////////////
// Return the next pixel from the image
// <DP> fix for 1 & 4 bpp images
int CImgGifFile::GifNextPixel( )
{
    if( CountDown == 0 ) return EOF;
    --CountDown;
    int r = m_imgData->getPixelIndex(curx,cury);
    // Bump the current X position
    ++curx;
    if( curx == m_imgData->width ){
        curx = 0;
        cury--;                 //bottom to top
    }
    return r;
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::Putword(int w, IILIO *io)
{
    fwritec(io, (uint8_t)(w & 0xff));
    fwritec(io, (uint8_t)((w / 256) & 0xff));
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::compressNONE( int init_bits, IILIO *io)
{
    long c;
    long ent;

    // g_init_bits - initial number of bits
    // g_ioOut   - pointer to output file
    g_init_bits = init_bits;
    g_ioOut = io;

     // Set up the necessary values
    cur_accum = cur_bits = clear_flg = 0;
    maxcode = (short)MAXCODE(n_bits = g_init_bits);
    code_int maxmaxcode = (code_int)1 << MAXBITSCODES;

    ClearCode = (1 << (init_bits - 1));
    EOFCode = ClearCode + 1;
    free_ent = (short)(ClearCode + 2);

    a_count=0;
    ent = GifNextPixel( );

    output( (code_int)ClearCode );

    while ( ent != EOF ) {    
        c = GifNextPixel();

        output ( (code_int) ent );
        ent = c;
        if ( free_ent < maxmaxcode ) {  
            free_ent++;
        } else {
            free_ent=(short)(ClearCode+2);
            clear_flg=1;
            output((code_int)ClearCode);
        }
    }
     // Put out the final code.
    output( (code_int) EOFCode );
}
////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 *
 *  GIFCOMPR.C       -     LZW GIF Image compression routines
 *
 ***************************************************************************/

void CImgGifFile::compressLZW( int init_bits, IILIO *io)
{
    long fcode;
    long c;
    long ent;
    long hshift;
    long disp;
    long i;

    // g_init_bits - initial number of bits
    // g_ioOut   - pointer to output file
    g_init_bits = init_bits;
    g_ioOut = io;

     // Set up the necessary values
    cur_accum = cur_bits = clear_flg = 0;
    maxcode = (short)MAXCODE(n_bits = g_init_bits);
    code_int maxmaxcode = (code_int)1 << MAXBITSCODES;

    ClearCode = (1 << (init_bits - 1));
    EOFCode = ClearCode + 1;
    free_ent = (short)(ClearCode + 2);

    a_count=0;
    ent = GifNextPixel( );

    hshift = 0;
    for ( fcode = (long) HSIZE;  fcode < 65536L; fcode *= 2L )    ++hshift;
    hshift = 8 - hshift;                /* set hash code range bound */
    cl_hash((long)HSIZE);        /* clear hash table */
    output( (code_int)ClearCode );

    while ( (c = GifNextPixel( )) != EOF ) {    

        fcode = (long) (((long) c << MAXBITSCODES) + ent);
        i = (((code_int)c << hshift) ^ ent);    /* xor hashing */

        if ( HashTabOf (i) == fcode ) {
            ent = CodeTabOf (i);
            continue;
        } else if ( (long)HashTabOf (i) < 0 )      /* empty slot */
            goto nomatch;
        disp = HSIZE - i;           /* secondary hash (after G. Knott) */
        if ( i == 0 )    disp = 1;
probe:
        if ( (i -= disp) < 0 )    i += HSIZE;
        if ( HashTabOf (i) == fcode ) {    ent = CodeTabOf (i); continue; }
        if ( (long)HashTabOf (i) > 0 )    goto probe;
nomatch:
        output ( (code_int) ent );
        ent = c;
        if ( free_ent < maxmaxcode ) {  
            CodeTabOf (i) = free_ent++; /* code -> hashtable */
            HashTabOf (i) = fcode;
        } else {
            cl_hash((long)HSIZE);
            free_ent=(short)(ClearCode+2);
            clear_flg=1;
            output((code_int)ClearCode);
        }
    }
     // Put out the final code.
    output( (code_int)ent );
    output( (code_int) EOFCode );
}
////////////////////////////////////////////////////////////////////////////////

static const unsigned long code_mask[] = {
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
    0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF,
    0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::output( code_int  code)
{
    cur_accum &= code_mask[ cur_bits ];

    if( cur_bits > 0 )
        cur_accum |= ((long)code << cur_bits);
    else
        cur_accum = code;

    cur_bits += n_bits;

    while( cur_bits >= 8 ) {
        char_out( (unsigned int)(cur_accum & 0xff) );
        cur_accum >>= 8;
        cur_bits -= 8;
    }

    /*
     * If the next entry is going to be too big for the code size,
     * then increase it, if possible.
     */

    if ( free_ent > maxcode || clear_flg ) {
        if( clear_flg ) {
            maxcode = (short)MAXCODE(n_bits = g_init_bits);
            clear_flg = 0;
        } else {
            ++n_bits;
            if ( n_bits == MAXBITSCODES )
                maxcode = (code_int)1 << MAXBITSCODES; /* should NEVER generate this code */
            else
                maxcode = (short)MAXCODE(n_bits);
        }
    }
    
    if( code == EOFCode ) {
         // At EOF, write the rest of the buffer.
        while( cur_bits > 0 ) {
            char_out( (unsigned int)(cur_accum & 0xff) );
            cur_accum >>= 8;
            cur_bits -= 8;
        }
    
        flush_char();

//         if(g_ioOut->Error())
//             strcpy(info.szLastError,"write Error in GIF file");
    }
}
////////////////////////////////////////////////////////////////////////////////

void CImgGifFile::cl_hash(long hsize)

{
    long *htab_p = htab+hsize;

    long i;
    long m1 = -1L;

    i = hsize - 16;

    do {
        *(htab_p-16)=m1;
        *(htab_p-15)=m1;
        *(htab_p-14)=m1;
        *(htab_p-13)=m1;
        *(htab_p-12)=m1;
        *(htab_p-11)=m1;
        *(htab_p-10)=m1;
        *(htab_p-9)=m1;
        *(htab_p-8)=m1;
        *(htab_p-7)=m1;
        *(htab_p-6)=m1;
        *(htab_p-5)=m1;
        *(htab_p-4)=m1;
        *(htab_p-3)=m1;
        *(htab_p-2)=m1;
        *(htab_p-1)=m1;
        
        htab_p-=16;
    } while ((i-=16) >=0);

    for (i+=16;i>0;--i)
        *--htab_p=m1;
}

/*******************************************************************************
*   GIF specific
*******************************************************************************/

void CImgGifFile::char_out(int c)
{
    accum[a_count++]=(char)c;
    if (a_count >=254)
        flush_char();
}

void CImgGifFile::flush_char()
{
    if (a_count > 0) {
        fwritec(g_ioOut, (uint8_t)a_count);
        g_ioOut->write(accum, a_count);
        a_count=0;
    }
}

/*******************************************************************************
*   GIF decoder
*******************************************************************************/
/* DECODE.C - An LZW decoder for GIF
 * Copyright (C) 1987, by Steven A. Bennett
 * Copyright (C) 1994, C++ version by Alejandro Aguilar Sierra
*
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 *
 * In accordance with the above, I want to credit Steve Wilhite who wrote
 * the code which this is heavily inspired by...
 *
 * GIF and 'Graphics Interchange Format' are trademarks (tm) of
 * Compuserve, Incorporated, an H&R Block Company.
 *
 * Release Notes: This file contains a decoder routine for GIF images
 * which is similar, structurally, to the original routine by Steve Wilhite.
 * It is, however, somewhat noticably faster in most cases.
 *
 */

////////////////////////////////////////////////////////////////////////////////

short CImgGifFile::init_exp(short size)
{
    curr_size = (short)(size + 1);
    top_slot = (short)(1 << curr_size);
    clear = (short)(1 << size);
    ending = (short)(clear + 1);
    slot = newcodes = (short)(ending + 1);
    navail_bytes = nbits_left = 0;

    memset(stack,0,MAX_CODES + 1);
    memset(prefix,0,MAX_CODES + 1);
    memset(suffix,0,MAX_CODES + 1);
    return(0);
}
////////////////////////////////////////////////////////////////////////////////

/* get_next_code()
 * - gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors...
 */
short CImgGifFile::get_next_code(IILIO *io)
{
    short i, x;
    uint32_t ret;

    if (nbits_left == 0) {
        if (navail_bytes <= 0) {
            /* Out of bytes in current block, so read next block */
            pbytes = byte_buff;
            if ((navail_bytes = (short)get_byte(io)) < 0)
                return(navail_bytes);
            else if (navail_bytes) {
                for (i = 0; i < navail_bytes; ++i) {
                    if ((x = (short)get_byte(io)) < 0) return(x);
                    byte_buff[i] = (uint8_t)x;
                }
            }
        }
        b1 = *pbytes++;
        nbits_left = 8;
        --navail_bytes;
    }

    if (navail_bytes<0) return ending; // prevent deadlocks (thanks to Mike Melnikov)

    ret = b1 >> (8 - nbits_left);
    while (curr_size > nbits_left){
        if (navail_bytes <= 0){
            /* Out of bytes in current block, so read next block*/
            pbytes = byte_buff;
            if ((navail_bytes = (short)get_byte(io)) < 0)
                return(navail_bytes);
            else if (navail_bytes){
                for (i = 0; i < navail_bytes; ++i){
                    if ((x = (short)get_byte(io)) < 0) return(x);
                    byte_buff[i] = (uint8_t)x;
                }
            }
        }
        b1 = *pbytes++;
        ret |= b1 << nbits_left;
        nbits_left += 8;
        --navail_bytes;
    }
    nbits_left = (short)(nbits_left-curr_size);
    ret &= code_mask[curr_size];
    return((short)(ret));
}
////////////////////////////////////////////////////////////////////////////////

/* short decoder(linewidth)
 *    short linewidth;               * Pixels per line of image *
 *
 * - This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to out_line(), which is a user specific function
 * to display a line of pixels.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * seperating them into the proper size codes.  Finally, get_byte() is
 * the global routine to read the next uint8_t from the GIF file.
 *
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 *
 * Returns: 0 if successful, else negative.  (See ERRS.H)
 *
 */
/* bad_code_count is incremented each time an out of range code is read.
 * When this value is non-zero after a decode, your GIF file is probably
 * corrupt in some way...
 */
short CImgGifFile::decoder(IILIO *io, CImageIterator* iter, short linewidth, int &bad_code_count)
{
    uint8_t *sp, *bufptr;
    uint8_t *buf;
    short code, fc, oc, bufcnt;
    short c, size, ret;

    /* Initialize for decoding a new image... */
    bad_code_count = 0;
    if ((size = (short)get_byte(io)) < 0)    return(size);
    if (size < 2 || 9 < size)                return(BAD_CODE_SIZE);
    // out_line = outline;
    init_exp(size);
    //printf("L %d %x\n",linewidth,size);

    /* Initialize in case they forgot to put in a clear code.
     * (This shouldn't happen, but we'll try and decode it anyway...)
     */
    oc = fc = 0;

   /* Allocate space for the decode buffer */
    if ((buf = new uint8_t[linewidth + 1]) == nullptr) return(OUT_OF_MEMORY);

   /* Set up the stack pointer and decode buffer pointer */
    sp = stack;
    bufptr = buf;
    bufcnt = linewidth;

   /* This is the main loop.  For each code we get we pass through the
    * linked list of prefix codes, pushing the corresponding "character" for
    * each code onto the stack.  When the list reaches a single "character"
    * we push that on the stack too, and then start unstacking each
    * character for output in the correct order.  Special handling is
    * included for the clear code, and the whole thing ends when we get
    * an ending code.
    */
    while ((c = get_next_code(io)) != ending) {
        /* If we had a file error, return without completing the decode*/
        if (c < 0){
            delete[] buf;
            return(0);
        }
        /* If the code is a clear code, reinitialize all necessary items.*/
        if (c == clear){
            curr_size = (short)(size + 1);
            slot = newcodes;
            top_slot = (short)(1 << curr_size);

            /* Continue reading codes until we get a non-clear code
            * (Another unlikely, but possible case...)
            */
            while ((c = get_next_code(io)) == clear);

            /* If we get an ending code immediately after a clear code
            * (Yet another unlikely case), then break out of the loop.
            */
            if (c == ending) break;

            /* Finally, if the code is beyond the range of already set codes,
            * (This one had better NOT happen...  I have no idea what will
            * result from this, but I doubt it will look good...) then set it
            * to color zero.
            */
            if (c >= slot) c = 0;
            oc = fc = c;

            /* And let us not forget to put the char into the buffer... And
            * if, on the off chance, we were exactly one pixel from the end
            * of the line, we have to send the buffer to the out_line()
            * routine...
            */
            *bufptr++ = (uint8_t)c;
            if (--bufcnt == 0) {
                if ((ret = (short)out_line(iter, buf, linewidth)) < 0) {
                    delete[] buf;
                    return(ret);
                }
                bufptr = buf;
                bufcnt = linewidth;
            }
        } else {
            /* In this case, it's not a clear code or an ending code, so
            * it must be a code code...  So we can now decode the code into
            * a stack of character codes. (Clear as mud, right?)
            */
            code = c;

            /* Here we go again with one of those off chances...  If, on the
            * off chance, the code we got is beyond the range of those already
            * set up (Another thing which had better NOT happen...) we trick
            * the decoder into thinking it actually got the last code read.
            * (Hmmn... I'm not sure why this works...  But it does...)
            */
            if (code >= slot) {
                if (code > slot) ++bad_code_count;
                code = oc;
                *sp++ = (uint8_t)fc;
            }

            /* Here we scan back along the linked list of prefixes, pushing
            * helpless characters (ie. suffixes) onto the stack as we do so.
            */
            while (code >= newcodes) {
                *sp++ = suffix[code];
                code = prefix[code];
            }

            /* Push the last character on the stack, and set up the new
            * prefix and suffix, and if the required slot number is greater
            * than that allowed by the current bit size, increase the bit
            * size.  (NOTE - If we are all full, we *don't* save the new
            * suffix and prefix...  I'm not certain if this is correct...
            * it might be more proper to overwrite the last code...
            */
            *sp++ = (uint8_t)code;
            if (slot < top_slot){
                suffix[slot] = (uint8_t)(fc = (uint8_t)code);
                prefix[slot++] = oc;
                oc = c;
            }
            if (slot >= top_slot){
                if (curr_size < 12) {
                    top_slot <<= 1;
                    ++curr_size;
                }
            }

            /* Now that we've pushed the decoded string (in reverse order)
            * onto the stack, lets pop it off and put it into our decode
            * buffer...  And when the decode buffer is full, write another
            * line...
            */
            while (sp > stack) {
                *bufptr++ = *(--sp);
                if (--bufcnt == 0) {
                    if ((ret = (short)out_line(iter, buf, linewidth)) < 0) {
                        delete[] buf;
                        return(ret);
                    }
                    bufptr = buf;
                    bufcnt = linewidth;
                }
            }
        }
    }
    ret = 0;
    if (bufcnt != linewidth)
        ret = (short)out_line(iter, buf, (linewidth - bufcnt));
    delete[] buf;
    return(ret);
}
////////////////////////////////////////////////////////////////////////////////
int CImgGifFile::get_num_frames(IILIO *io,struct_TabCol* TabColSrc)
{
    struct_image image;

    size_t pos= io->getPos();
    int nframes=0;

    struct_TabCol TempTabCol;
    memcpy(&TempTabCol,TabColSrc,sizeof(struct_TabCol));

    char ch;
    for (bool bContinue = true; bContinue; ) {
        if (io->read(&ch, sizeof(ch)) != sizeof(ch)) {break;}
        // if (fp->read(&ch, sizeof(ch), 1) != 1) {break;}

        switch (ch)
        {
        case '!': // extension
            {
            decodeExtension(io);
            break;
            }
        case ',': // image
            {

            nframes++;

            assert(sizeof(image) == 9);
            //log << "Image header" << endl;
            // fp->read(&image,sizeof(image),1);
            io->read(&image, sizeof(image));


            //avoid uint8_t order problems with Solaris <candan>
            uint8_t *byteData = (uint8_t *) & image;
            image.l = byteData[0]+byteData[1]*256;
            image.t = byteData[2]+byteData[3]*256;
            image.w = byteData[4]+byteData[5]*256;
            image.h = byteData[6]+byteData[7]*256;

            // Local colour map?
            if (image.pf & 0x80) {
                TempTabCol.sogct = (short)(1 << ((image.pf & 0x07) +1));
                assert(3 == sizeof(struct rgb_color));
                // fp->read(TempTabCol.paleta,sizeof(struct rgb_color)*TempTabCol.sogct,1);
                io->read(TempTabCol.paleta, sizeof(struct rgb_color)*TempTabCol.sogct);
                //log << "Local colour map" << endl;
            }

            int bpp; //<DP> select the correct bit per pixel value
            if        (TempTabCol.sogct <= 2)  bpp = 1;
            else if (TempTabCol.sogct <= 16) bpp = 4;
            else                         bpp = 8;

            m_imgData->create(image.w, image.h, bpp);

            CImageIterator* iter = new CImageIterator(m_imgData);
            iter->upset();
            int badcode;
            ibf = GIFBUFTAM+1;

            interlaced = image.pf & 0x40;
            iheight = image.h;
            istep = 8;
            iypos = 0;
            ipass = 0;

            //if (interlaced) log << "Interlaced" << endl;
            decoder(io, iter, image.w, badcode);
            delete iter;
            io->seek(-(ibfmax - ibf - 1), SEEK_CUR);
            break;
            }
        case ';': //terminator
            bContinue=false;
            break;
        default:
            break;
        }
    }

    io->seek((int)pos, SEEK_SET);
    return nframes;
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::setDisposalMethod(int dm)
{    m_dispmeth=dm; }
////////////////////////////////////////////////////////////////////////////////
long CImgGifFile::getDisposalMethod()
{    return m_dispmeth; }
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::setLoops(int loops)
{    m_loops=loops; }
////////////////////////////////////////////////////////////////////////////////
long CImgGifFile::getLoops()
{    return m_loops; }
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::setComment(const char* sz_comment_in)
{    if (sz_comment_in) strncpy(m_comment,sz_comment_in,255); }
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::getComment(char* sz_comment_out)
{    if (sz_comment_out) strncpy(sz_comment_out,m_comment,255); }

////////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue 
 * Software Corporation disclaims all warranties, either express or implied, 
 * including but not limited to implied warranties of merchantability and 
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation. 
 * 
 * The miGIF compression routines do not, strictly speaking, generate files 
 * conforming to the GIF spec, since the image data is not LZW-compressed 
 * (this is the point: in order to avoid transgression of the Unisys patent 
 * on the LZW algorithm.)  However, miGIF generates data streams that any 
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs 
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid 
 * shapes on a solid-colored background. It gives little or no compression 
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 */
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_clear(struct_RLE* rle)
{
    rle->out_bits = rle->out_bits_init;
    rle->out_bump = rle->out_bump_init;
    rle->out_clear = rle->out_clear_init;
    rle->out_count = 0;
    rle->rl_table_max = 0;
    rle->just_cleared = 1;
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_flush(struct_RLE* rle)
{
    if (rle->rl_count == 1){
        rle_output_plain(rle->rl_pixel,rle);
        rle->rl_count = 0;
        return;
    }
    if (rle->just_cleared){
        rle_flush_fromclear(rle->rl_count,rle);
    } else if ((rle->rl_table_max < 2) || (rle->rl_table_pixel != rle->rl_pixel)) {
        rle_flush_clearorrep(rle->rl_count,rle);
    } else {
        rle_flush_withtable(rle->rl_count,rle);
    }
    rle->rl_count = 0;
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_output_plain(int c,struct_RLE* rle)
{
    rle->just_cleared = 0;
    rle_output(c,rle);
    rle->out_count++;
    if (rle->out_count >= rle->out_bump){
        rle->out_bits ++;
        rle->out_bump += 1 << (rle->out_bits - 1);
    }
    if (rle->out_count >= rle->out_clear){
        rle_output(rle->code_clear,rle);
        rle_clear(rle);
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_flush_fromclear(int count,struct_RLE* rle)
{
    int n;

    rle->out_clear = rle->max_ocodes;
    rle->rl_table_pixel = rle->rl_pixel;
    n = 1;
    while (count > 0){
        if (n == 1){
            rle->rl_table_max = 1;
            rle_output_plain(rle->rl_pixel,rle);
            count --;
        } else if (count >= n){
            rle->rl_table_max = n;
            rle_output_plain(rle->rl_basecode+n-2,rle);
            count -= n;
        } else if (count == 1){
            rle->rl_table_max ++;
            rle_output_plain(rle->rl_pixel,rle);
            count = 0;
        } else {
            rle->rl_table_max ++;
            rle_output_plain(rle->rl_basecode+count-2,rle);
            count = 0;
        }
        if (rle->out_count == 0) n = 1; else n ++;
    }
    rle_reset_out_clear(rle);
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_reset_out_clear(struct_RLE* rle)
{
    rle->out_clear = rle->out_clear_init;
    if (rle->out_count >= rle->out_clear){
        rle_output(rle->code_clear,rle);
        rle_clear(rle);
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_flush_withtable(int count, struct_RLE* rle)
{
    int repmax;
    int repleft;
    int leftover;

    repmax = count / rle->rl_table_max;
    leftover = count % rle->rl_table_max;
    repleft = (leftover ? 1 : 0);
    if (rle->out_count+repmax+repleft > rle->max_ocodes){
        repmax = rle->max_ocodes - rle->out_count;
        leftover = count - (repmax * rle->rl_table_max);
        repleft = 1 + rle_compute_triangle_count(leftover,rle->max_ocodes);
    }
    if (1+rle_compute_triangle_count(count,rle->max_ocodes) < (unsigned int)(repmax+repleft)){
        rle_output(rle->code_clear,rle);
        rle_clear(rle);
        rle_flush_fromclear(count,rle);
        return;
    }
    rle->out_clear = rle->max_ocodes;
    for (;repmax>0;repmax--) rle_output_plain(rle->rl_basecode+rle->rl_table_max-2,rle);
    if (leftover){
        if (rle->just_cleared){
            rle_flush_fromclear(leftover,rle);
        } else if (leftover == 1){
            rle_output_plain(rle->rl_pixel,rle);
        } else {
            rle_output_plain(rle->rl_basecode+leftover-2,rle);
        }
    }
    rle_reset_out_clear(rle);
}
////////////////////////////////////////////////////////////////////////////////
unsigned int CImgGifFile::rle_compute_triangle_count(unsigned int count, unsigned int nrepcodes)
{
    unsigned int perrep;
    unsigned int cost;

    cost = 0;
    perrep = (nrepcodes * (nrepcodes+1)) / 2;
    while (count >= perrep){
        cost += nrepcodes;
        count -= perrep;
    }
    if (count > 0){
        unsigned int n;
        n = rle_isqrt(count);
        while ((n*(n+1)) >= 2*count) n --;
        while ((n*(n+1)) < 2*count) n ++;
        cost += n;
    }
    return(cost);
}
////////////////////////////////////////////////////////////////////////////////
unsigned int CImgGifFile::rle_isqrt(unsigned int x)
{
    unsigned int r;
    unsigned int v;

    if (x < 2) return(x);
    for (v=x,r=1;v;v>>=2,r<<=1) ;
    while (1){
        v = ((x / r) + r) / 2;
        if ((v == r) || (v == r+1)) return(r);
        r = v;
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_flush_clearorrep(int count, struct_RLE* rle)
{
    int withclr;
    withclr = 1 + rle_compute_triangle_count(count,rle->max_ocodes);
    if (withclr < count) {
        rle_output(rle->code_clear,rle);
        rle_clear(rle);
        rle_flush_fromclear(count,rle);
    } else {
        for (;count>0;count--) rle_output_plain(rle->rl_pixel,rle);
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_write_block(struct_RLE* rle)
{
    fwritec(g_ioOut, (uint8_t)rle->oblen);

    g_ioOut->write(rle->oblock, rle->oblen);
    rle->oblen = 0;
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_block_out(unsigned char c, struct_RLE* rle)
{
    rle->oblock[rle->oblen++] = c;
    if (rle->oblen >= 255) rle_write_block(rle);
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_block_flush(struct_RLE* rle)
{
    if (rle->oblen > 0) rle_write_block(rle);
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_output(int val, struct_RLE* rle)
{
    rle->obuf |= val << rle->obits;
    rle->obits += rle->out_bits;
    while (rle->obits >= 8){
        rle_block_out(rle->obuf&0xff,rle);
        rle->obuf >>= 8;
        rle->obits -= 8;
    }
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::rle_output_flush(struct_RLE* rle)
{
     if (rle->obits > 0) rle_block_out(rle->obuf,rle);
     rle_block_flush(rle);
}
////////////////////////////////////////////////////////////////////////////////
void CImgGifFile::compressRLE( int init_bits, IILIO *io)
{
    g_init_bits = init_bits;
    g_ioOut = io;

    struct_RLE rle;
    rle.code_clear = 1 << (init_bits - 1);
    rle.code_eof = rle.code_clear + 1;
    rle.rl_basecode = rle.code_eof + 1;
    rle.out_bump_init = (1 << (init_bits - 1)) - 1;
    rle.out_clear_init = (init_bits <= 3) ? 9 : (rle.out_bump_init-1);
    rle.out_bits_init = init_bits;
    rle.max_ocodes = (1 << MAXBITSCODES) - ((1 << (rle.out_bits_init - 1)) + 3);
    rle.rl_count = 0;
    rle_clear(&rle);
    rle.obuf = 0;
    rle.obits = 0;
    rle.oblen = 0;

    rle_output(rle.code_clear,&rle);

    int c;
    while (1){
        c = GifNextPixel();
        if ((rle.rl_count > 0) && (c != rle.rl_pixel)) rle_flush(&rle);
        if (c == EOF) break;
        if (rle.rl_pixel == c){
            rle.rl_count++;
        } else {
            rle.rl_pixel = c;
            rle.rl_count = 1;
        }
    }
    rle_output(rle.code_eof,&rle);
    rle_output_flush(&rle);
}
