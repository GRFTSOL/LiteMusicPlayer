#include "RawImageData.h"
#include "ILIO.h"

//#define PNG_EXPORTA(ordinal, type, name, args, attributes) type name args

extern "C" {
#include "./png/pngpriv.h"
#include "./png/pngstruct.h"
#include "./png/pnginfo.h"
}


// cexcept interface

static void
png_cexcept_error(png_structp png_ptr, png_const_charp msg) {
    ERR_LOG1("libpng error: %s", msg);
}

void preMultiplyRGBChannels(RawImageData *imgData) {
    assert(imgData->pixFormat == PF_RGBA32);

    // pre-multiply rgb channels with alpha channel
    uint8_t *byPixel, *byRow;

    byRow = imgData->rowPtr(0);
    for (int y = 0; y < imgData->height; y++) {
        byPixel = byRow;
        for (int x = 0; x < imgData->width; x++) {
            byPixel[0] = byPixel[0] * byPixel[3] / 255;
            byPixel[1] = byPixel[1] * byPixel[3] / 255;
            byPixel[2] = byPixel[2] * byPixel[3] / 255;
            byPixel += 4;
        }
        byRow += imgData->stride;
    }
}

static void PNGAPI png_read_data_x(png_structp png_ptr, png_bytep data, png_size_t length) {
    IILIO *io;

    io = (IILIO*)png_ptr->io_ptr;

    if (io->read(data, length) != length) {
        png_error(png_ptr, "read Error");
    }
}

RawImageDataPtr loadRawImageDataFromPngFile(IILIO *io) {
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    png_byte pbSig[8];
    int iBitDepth;
    int iColorType;
    double dGamma;
    png_color_16 *pBackground;
    png_uint_32 ulChannels;
    png_byte **ppbRowPointers = nullptr;
    unsigned long i;
    png_uint_32 nWidth, nHeight;
    RawImageDataPtr imgData;
    png_bytep pRow;

    // first check the eight uint8_t PNG signature

    if (io->read(pbSig, 8) != 8) {
        goto R_FAILED;
    }

    // io->seek(0, SEEK_SET);
    if (!png_check_sig(pbSig, 8)) {
        goto R_FAILED;
    }

    // create the two png(-info) structures

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
        (png_error_ptr)png_cexcept_error, (png_error_ptr)nullptr);
    if (!png_ptr) {
        goto R_FAILED;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        goto R_FAILED;
    }

    imgData = make_shared<RawImageData>();

    //     __try
    //     {
    // initialize the png structure
    // png_init_io(png_ptr, pfFile);
    png_set_read_fn(png_ptr, io, png_read_data_x);

    png_set_sig_bytes(png_ptr, 8);

    // read all PNG info up to image data
    png_read_info(png_ptr, info_ptr);

    // get width, height, bit-depth and color-type
    png_get_IHDR(png_ptr, info_ptr, &nWidth, &nHeight, &iBitDepth,
        &iColorType, nullptr, nullptr, nullptr);

    // expand images of all color-type and bit-depth to 3x8 bit RGB images
    // let the library process things like alpha, transparency, background
    if (iBitDepth == 16) {
        png_set_strip_16(png_ptr);
    }
    if (iColorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_expand(png_ptr);
    }
    if (iBitDepth < 8) {
        png_set_expand(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_expand(png_ptr);
    }
    if (iColorType == PNG_COLOR_TYPE_GRAY ||
        iColorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    png_set_expand(png_ptr);

    // <vho> - flip the RGB pixels to BGR (or RGBA to BGRA)
#ifdef _WIN32
    if (info_ptr->color_type & PNG_COLOR_MASK_COLOR) {
        png_set_bgr(png_ptr);
    }
#endif

    // set the background color to draw transparent and alpha images over.
    if (png_get_bKGD(png_ptr, info_ptr, &pBackground)) {
        png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    }

    // if required set gamma conversion
    if (png_get_gAMA(png_ptr, info_ptr, &dGamma)) {
        png_set_gamma(png_ptr, (double) 2.2, dGamma);
    }

    // after the transformations have been registered update info_ptr data

    png_read_update_info(png_ptr, info_ptr);

    // get again width, height and the new bit-depth and color-type

    png_get_IHDR(png_ptr, info_ptr, &nWidth, &nHeight, &iBitDepth,
        &iColorType, nullptr, nullptr, nullptr);

    // row_bytes is the width x number of channels
    /*png_uint_32 ulRowBytes = */png_get_rowbytes(png_ptr, info_ptr);
    ulChannels = png_get_channels(png_ptr, info_ptr);

    // now we can allocate memory to store the image
    if (!imgData->create(nWidth, nHeight, ulChannels == 4 ? 32 : 24)) {
        goto R_FAILED;
    }

    // memset(imgData->buff, 0, imgData->getBuffSize());

    // and allocate memory for an array of row-pointers
    if ((ppbRowPointers = (png_bytepp) malloc(nHeight
        * sizeof(png_bytep))) == nullptr) {
        png_error(png_ptr, "Visual PNG: out of memory");
        goto R_FAILED;
    }

    if (imgData->stride >= 0) {
        pRow = imgData->buff;
    } else {
        pRow = imgData->buff - int(imgData->height - 1) * imgData->stride;
    }
    // set the individual row-pointers to point at the correct offsets
    for (i = 0; i < nHeight; i++) {
        ppbRowPointers[i] = pRow;
        pRow += imgData->stride;
    }

    // now we can go ahead and just read the whole image
    png_read_image(png_ptr, ppbRowPointers);

    if (ulChannels == 4) {
        preMultiplyRGBChannels(imgData.get());
    }

    // read the additional chunks in the PNG file (not really needed)
    png_read_end(png_ptr, nullptr);

    png_destroy_info_struct(png_ptr, &info_ptr);

    png_destroy_read_struct(&png_ptr, nullptr, nullptr);

    // and we're done
    free (ppbRowPointers);
    ppbRowPointers = nullptr;
    //     }
    //     __except(EXCEPTION_EXECUTE_HANDLER)
    //     {
    //         if(ppbRowPointers)
    //             free (ppbRowPointers);
    //
    //         goto R_FAILED;
    //     }

    return imgData;

R_FAILED:
    if (png_ptr) {
        if (info_ptr) {
            png_destroy_info_struct(png_ptr, &info_ptr);
        }
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    }

    return nullptr;
}

//
// bool CImgPngFile::save(cstr_t szFile)
// {
//     // uint8_t        trans[256];    //for transparency (don't move)
//     png_struct    *png_ptr;
//     png_info    *info_ptr;
//     FILE        *fp;
//
//     // open the file
//     fp = fopen((const char *)szFile, "wb");
//     if (!fp)
//         return false;
//
//
//     try
//     {
//         /* create and initialize the png_struct with the desired error handler
//         * functions.  If you want to use the default stderr and longjump method,
//         * you can supply nullptr for the last three parameters.  We also check that
//         * the library version is compatible with the one used at compile time,
//         * in case we are using dynamically linked libraries.  REQUIRED.
//         */
//         png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,(void *)nullptr,nullptr,nullptr);
//         if (png_ptr == nullptr) throw "Failed to create PNG structure";
//
//         /* Allocate/initialize the image information data.  REQUIRED */
//         info_ptr = png_create_info_struct(png_ptr);
//         if (info_ptr == nullptr){
//             png_destroy_write_struct(&png_ptr,  (png_infopp)nullptr);
//             throw "Failed to initialize PNG info structure";
//         }
//
//         /* Set error handling.  REQUIRED if you aren't supplying your own
//         * error hadnling functions in the png_create_write_struct() call.
//         */
//         if (setjmp(png_ptr->jmpbuf)){
//             /* If we get here, we had a problem reading the file */
//             if (info_ptr->palette) free(info_ptr->palette);
//             png_destroy_write_struct(&png_ptr,  (png_infopp)&info_ptr);
//             throw "Error saving PNG file";
//         }
//
//         // int row_stride = info.dwEffWidth;
//         int row_stride = (m_lpImgData->GetWidth() * m_lpImgData->GetDepth() + 7) >> 3;
//         /* set up the output control */
//         png_init_io(png_ptr, fp);
//         // use custom I/O functions
//         // png_set_write_fn(png_ptr,hFile,(png_rw_ptr)user_write_data,(png_flush_ptr)user_flush_data);
//
//         /* set the file information here */
//         info_ptr->width = m_lpImgData->GetWidth();
//         info_ptr->height = m_lpImgData->GetHeight();
//         info_ptr->pixel_depth = (uint8_t)m_lpImgData->GetDepth();
//         info_ptr->channels = (m_lpImgData->GetDepth()>8) ? (uint8_t)3: (uint8_t)1;
//         info_ptr->bit_depth = (uint8_t)(m_lpImgData->GetDepth()/info_ptr->channels);
//         info_ptr->color_type = m_lpImgData->GetColorType();
//         info_ptr->compression_type = info_ptr->filter_type = info_ptr->interlace_type=0;
//         info_ptr->valid = 0;
//         info_ptr->interlace_type=PNG_INTERLACE_NONE;
//         info_ptr->rowbytes = row_stride;
//
//         /* set the palette if there is one */
//         if ((m_lpImgData->GetColorType() & COLORTYPE_PALETTE) && m_lpImgData->GetPalette()){
//             png_set_IHDR(png_ptr, info_ptr, info_ptr->width, info_ptr->height, info_ptr->bit_depth,
//                 PNG_COLOR_TYPE_PALETTE, info_ptr->interlace_type,
//                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
//             info_ptr->valid |= PNG_INFO_PLTE;
//
//             //<DP> simple transparency
// //            if (info.nBkgndIndex != -1){
// //                trans[0]=0;
// //                info_ptr->num_trans = 1;
// //                info_ptr->valid |= PNG_INFO_tRNS;
// //                info_ptr->trans = trans;
// //                // the transparency indexes start from 0
// //                if (info.nBkgndIndex){
// //                    SwapIndex(0,(uint8_t)info.nBkgndIndex);
// //                    // the ghost must set the changed attributes in the body
// //                    if (info.pGhost) ((CxImage*)info.pGhost)->setTransIndex(0);
// //                }
// //            }
// //            if (info.bAlphaPaletteEnabled){
// //                for(uint16_t ip=0; ip<head.biClrUsed;ip++)
// //                    trans[ip]=getPaletteColor((uint8_t)ip).rgbReserved;
// //                info_ptr->num_trans = (uint16_t)head.biClrUsed;
// //                info_ptr->valid |= PNG_INFO_tRNS;
// //                info_ptr->trans = trans;
// //            }
//
//             // copy the palette colors
//             RGBQUAD        *pPalette;
//             int nc = m_lpImgData->GetPaletteItemCount();
//             info_ptr->palette = new png_color[nc];
//             info_ptr->num_palette = (png_uint_16) nc;
//             pPalette = m_lpImgData->GetPalette();
//             for (int i = 0; i < nc; i++)
//             {
//                 info_ptr->palette[i].red = pPalette[i].rgbRed;
//                 info_ptr->palette[i].blue = pPalette[i].rgbBlue;
//                 info_ptr->palette[i].green = pPalette[i].rgbGreen;
//             }
//         }
// //
// //#if CXIMAGE_SUPPORT_ALPHA    // <vho>
// //        //Merge the transparent color with the alpha channel
// //        bool bNeedTempAlpha = false;
// //        if (head.biBitCount==24 && info.nBkgndIndex>=0){
// //            if (!AlphaIsValid()){
// //                bNeedTempAlpha = true;
// //                AlphaCreate();
// //                AlphaSet(255);
// //            }
// //            RGBQUAD c,ct=getTransColor();
// //            for(long y=0; y < head.biHeight; y++){
// //                for(long x=0; x < head.biWidth ; x++){
// //                    c=getPixelColor(x,y);
// //                    if (*(long*)&c==*(long*)&ct)
// //                        AlphaSet(x,y,0);
// //            }}
// //        }
// //#endif // CXIMAGE_SUPPORT_ALPHA    // <vho>
// //
// //#if CXIMAGE_SUPPORT_ALPHA    // <vho>
// //        if (AlphaIsValid()){
// //            row_stride = 4 * head.biWidth;
// //
// //            info_ptr->pixel_depth = 32;
// //            info_ptr->channels = 4;
// //            info_ptr->bit_depth = 8;
// //            info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
// //            info_ptr->rowbytes = row_stride;
// //
// //            /* write the file information */
// //            png_write_info(png_ptr, info_ptr);
// //
// //            //<Ranger> "10+row_stride" fix heap deallocation problem during debug???
// //            uint8_t *row_pointers = new uint8_t[10+row_stride];
// //            iter.upset();
// //            long ay=head.biHeight-1;
// //            RGBQUAD c;
// //            do    {
// //                iter.getRow(row_pointers, row_stride);
// //                for (long ax=head.biWidth-1; ax>=0;ax--){
// //                    c=getPixelColor(ax,ay);
// //                    row_pointers[ax*4+3]=(uint8_t)((AlphaGet(ax,ay)*info.nAlphaMax)/255);
// //                    row_pointers[ax*4+2]=c.rgbBlue;
// //                    row_pointers[ax*4+1]=c.rgbGreen;
// //                    row_pointers[ax*4]=c.rgbRed;
// //                }
// //                png_write_row(png_ptr, row_pointers);
// //                ay--;
// //            } while(iter.prevRow());
// //
// //            delete [] row_pointers;
// //        }
// //        else
// //#endif //CXIMAGE_SUPPORT_ALPHA    // <vho>
//         {
//             /* write the file information */
//             png_write_info(png_ptr, info_ptr);
//             /* If you are only writing one row at a time, this works */
//             uint8_t *row_pointers = new uint8_t[10+row_stride];
//             for (int i = m_lpImgData->GetHeight() - 1; i >= 0; i--)
//             {
//                 uint8_t *pRow = m_lpImgData->GetImgRow(i);
//                 if (info_ptr->color_type == 2 /*COLORTYPE_COLOR*/)
//                 {
//                     m_lpImgData->RGBtoBGR(pRow, row_pointers, row_stride);
//                     // m_lpImgData->RGBtoBGR(pRow)
//                     png_write_row(png_ptr, row_pointers);
//                 }
//                 else
//                     png_write_row(png_ptr, pRow);
//             }
//             delete [] row_pointers;
//         }
// //
// //#if CXIMAGE_SUPPORT_ALPHA    // <vho>
// //        /* remove the temporary alpha channel*/
// //        if (bNeedTempAlpha) AlphaDelete();
// //#endif // CXIMAGE_SUPPORT_ALPHA    // <vho>
// //
//         /* It is REQUIRED to call this to finish writing the rest of the file */
//         png_write_end(png_ptr, info_ptr);
//
//         /* if you malloced the palette, free it here */
//         if (info_ptr->palette)    delete[] (info_ptr->palette);
//
//         /* clean up after the write, and free any memory allocated */
//         png_destroy_write_struct(&png_ptr, (png_infopp)&info_ptr);
//
//         } catch (char *message) {
//             ERR_LOG1("Error, PNG: %s", message);
//         // strncpy(info.szLastError,message,255);
//         return false;
//     }
//
//     /* close the file */
//     fclose(fp);
//
//     /* that's it */
//     return true;
// }
