#include "RawImageData.h"
#include "ILIO.h"
#include "ImgJpegFile.h"
#include <setjmp.h>


struct ima_error_mgr {
    struct jpeg_error_mgr pub;    /* "public" fields */
    jmp_buf setjmp_buffer;      /* for return to caller */
    char                        buffer[255];                /* error message <CSC>*/
};
typedef ima_error_mgr *ima_error_ptr;

void setGrayPalette(RGBQUAD *pal, int nClrUsed) {
    for (int ni = 0; ni < nClrUsed; ni++) {
        pal[ni].rgbBlue=pal[ni].rgbGreen = pal[ni].rgbRed = (uint8_t)(ni * (255 / (nClrUsed - 1)));
    }
}

void setPalette(RGBQUAD *pal, int n, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (!g) {
        g = r;
    }
    if (!b) {
        b = g;
    }

    for (int i = 0; i < n ; i++) {
        pal[i].rgbRed = r[i];
        pal[i].rgbGreen = g[i];
        pal[i].rgbBlue = b[i];
    }
}

////////////////////////////////////////////////////////////////////////////////
// Here's the routine that will replace the standard error_exit method:
////////////////////////////////////////////////////////////////////////////////
static void
ima_jpeg_error_exit (j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    ima_error_ptr myerr = (ima_error_ptr) cinfo->err;
    /* create the message */
    myerr->pub.format_message (cinfo, myerr->buffer);
    /* Send it to stderr, adding a newline */
    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}


RawImageDataPtr loadRawImageDataFromJpgFile(IILIO *io) {
    CxFileJpg src(io);
    RawImageDataPtr imgData;

    /* This struct contains the JPEG decompression parameters and pointers to
    * working space (which is allocated as needed by the JPEG library).
    */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler. <CSC> */
    struct ima_error_mgr jerr;
    // jerr.buffer=info.szLastError;
    /* More stuff */
    JSAMPARRAY buffer; /* Output row buffer */
    int row_stride; /* physical row width in output buffer */

    /* In this example we want to open the input file before doing anything else,
    * so that the setjmp() error recovery below can assume the file is open.
    * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
    * requires it in order to read binary files.
    */

    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ima_jpeg_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        jpeg_destroy_decompress(&cinfo);
        goto R_FAILED;
    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    //jpeg_stdio_src(&cinfo, infile);
    cinfo.src = &src;

    /* Step 3: read file parameters with jpeg_read_header() */
    (void) jpeg_read_header(&cinfo, true);

    //<DP>: Load true color images as RGB (no quantize)
    /* Step 4: set parameters for decompression */
    /*  if (cinfo.jpeg_color_space!=JCS_GRAYSCALE) {
 *    cinfo.quantize_colors = true;
 *    cinfo.desired_number_of_colors = 128;
 *}
 */ //</DP>

    /* Step 5: Start decompressor */
    jpeg_start_decompress(&cinfo);

    /* We may need to do some setup of our own at this point before reading
    * the data.  After jpeg_start_decompress() we have the correct scaled
    * output image dimensions available, as well as the output colormap
    * if we asked for color quantization.
    */
    imgData = make_shared<RawImageData>();
    if (!imgData->create(cinfo.image_width, cinfo.image_height, 8 * cinfo.num_components)) {
        goto R_FAILED;
    }
    //
    //    if (cinfo.density_unit==2){
    //        SetXDPI((254*cinfo.X_density)/100);
    //        SetYDPI((254*cinfo.Y_density)/100);
    //    } else {
    //        SetXDPI(cinfo.X_density);
    //        SetYDPI(cinfo.Y_density);
    //    }

    if (cinfo.jpeg_color_space==JCS_GRAYSCALE){
        imgData->setClrUsed(256);
        setGrayPalette(imgData->pallete, 256);
    } else {
        if (cinfo.quantize_colors == true){
            imgData->setClrUsed(cinfo.actual_number_of_colors);
            setPalette(imgData->pallete, cinfo.actual_number_of_colors, cinfo.colormap[0], cinfo.colormap[1], cinfo.colormap[2]);
        }
    }

    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.num_components;

    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    assert(row_stride <= imgData->absStride());

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */
    /* Here we use the library's state variable cinfo.output_scanline as the
    * loop counter, so that we don't have to keep track ourselves.
    */
    int y;
    y = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        // if (info.nEscape) longjmp(jerr.setjmp_buffer, 1); // <vho> - cancel decoding

        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        // info.nProgress = (long)(100*cinfo.output_scanline/cinfo.output_height);
        //<DP> Step 6a: CMYK->RGB */
        if ((cinfo.num_components==4)&&(cinfo.quantize_colors==false)){
            uint8_t k,*dst,*src;
            dst=imgData->rowPtr(y);
            src=buffer[0];
            for(long x3=0,x4=0; x3<(long)imgData->absStride() && x4<row_stride; x3+=3, x4+=4){
                k=src[x4+3];
                dst[x3] =(uint8_t)((k * src[x4+2])/255);
                dst[x3+1]=(uint8_t)((k * src[x4+1])/255);
                dst[x3+2]=(uint8_t)((k * src[x4+0])/255);
            }
        } else {
            /* Assume put_scanline_someplace wants a pointer and sample count. */
            memcpy(imgData->rowPtr(y), buffer[0], row_stride);
        }
        y++;
    }

    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    //<DP> Step 7A: Swap red and blue components */
#ifndef _MAC_OS
    if ((cinfo.num_components==3)&&(cinfo.quantize_colors==false)){
        uint8_t *r,*b,t,*r0;
        JDIMENSION x,y;
        // r0=GetBits();
        r0 = imgData->rowPtr(0);
        for(y=0;y<cinfo.image_height;y++){

            // if (info.nEscape) longjmp(jerr.setjmp_buffer, 1); // <vho> - cancel decoding

            r=r0; b=r+2;
            for(x=0;x<cinfo.image_width;x++){
                t=*r; *r=*b; *b=t; r+=3; b+=3;
            }
            r0+=imgData->stride;
        }
    } //</DP>
#endif

#if CXIMAGEJPG_SUPPORT_EXIF
    CxExifInfo info(&m_exifinfo);
    info.decodeInfo(cinfo);
#endif //CXIMAGEJPG_SUPPORT_EXIF

    /* Step 8: Release JPEG decompression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* At this point you may want to check to see whether any corrupt-data
    * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
    */

    /* And we're done! */
    return imgData;

R_FAILED:
    return nullptr;
}
//
// bool CImgJpegFile::save(cstr_t szFile)
// {
//     FILE        *hFile;
//
//     if (m_lpImgData->GetPaletteItemCount() != 0 && !m_lpImgData->IsGrayScale()){
//         // strcpy(info.szLastError,"JPEG can save only RGB or GreyScale images");
//         ERR_LOG0("JPEG can save only RGB or GreyScale images");
//         return false;
//     }
//
//     hFile = _tfopen(szFile, "wb");
//     if (!hFile)
//         return false;
//
//     /* This struct contains the JPEG compression parameters and pointers to
//     * working space (which is allocated as needed by the JPEG library).
//     * It is possible to have several such structures, representing multiple
//     * compression/decompression processes, in existence at once.  We refer
//     * to any one struct (and its associated working data) as a "JPEG object".
//     */
//     struct jpeg_compress_struct cinfo;
//     /* This struct represents a JPEG error handler.  It is declared separately
//     * because applications often want to supply a specialized error handler
//     * (see the second half of this file for an example).  But here we just
//     * take the easy way out and use the standard error handler, which will
//     * print a message on stderr and call exit() if compression fails.
//     * Note that this struct must live as long as the main JPEG parameter
//     * struct, to avoid dangling-pointer problems.
//     */
//     //struct jpeg_error_mgr jerr;
//     /* We use our private extension JPEG error handler. <CSC> */
//     struct ima_error_mgr jerr;
//     // jerr.buffer=info.szLastError;
//     /* More stuff */
//     int row_stride;        /* physical row width in image buffer */
//     JSAMPARRAY buffer;        /* Output row buffer */
//
//     /* Step 1: allocate and initialize JPEG compression object */
//     /* We have to set up the error handler first, in case the initialization
//     * step fails.  (Unlikely, but it could happen if you are out of memory.)
//     * This routine fills in the contents of struct jerr, and returns jerr's
//     * address which we place into the link field in cinfo.
//     */
//     //cinfo.err = jpeg_std_error(&jerr); <CSC>
//     /* We set up the normal JPEG error routines, then override error_exit. */
//     cinfo.err = jpeg_std_error(&jerr.pub);
//     jerr.pub.error_exit = ima_jpeg_error_exit;
//
//     /* Establish the setjmp return context for my_error_exit to use. */
//     if (setjmp(jerr.setjmp_buffer)) {
//         /* If we get here, the JPEG code has signaled an error.
//         * We need to clean up the JPEG object, close the input file, and return.
//         */
//         // strcpy(info.szLastError, jerr.buffer); //<CSC>
//         jpeg_destroy_compress(&cinfo);
//         return 0;
//     }
//
//     /* Now we can initialize the JPEG compression object. */
//     jpeg_create_compress(&cinfo);
//     /* Step 2: specify data destination (eg, a file) */
//     /* Note: steps 2 and 3 can be done in either order. */
//     /* Here we use the library-supplied code to send compressed data to a
//     * stdio stream.  You can also write your own code to do something else.
//     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
//     * requires it in order to write binary files.
//     */
//
//     //jpeg_stdio_dest(&cinfo, outfile);
//     CxFileJpg dest(hFile);
//     cinfo.dest = &dest;
//
//     /* Step 3: set parameters for compression */
//     /* First we supply a description of the input image.
//     * Four fields of the cinfo struct must be filled in:
//     */
//     cinfo.image_width = m_lpImgData->GetWidth();     // image width and height, in pixels
//     cinfo.image_height = m_lpImgData->GetHeight();
//
//     if (m_lpImgData->IsGrayScale()){
//         cinfo.input_components = 1;            // # of color components per pixel
//         cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
//     } else {
//         cinfo.input_components = 3;     // # of color components per pixel
//         cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
//     }
//
//     /* Now use the library's routine to set default compression parameters.
//     * (You must set at least cinfo.in_color_space before calling this,
//     * since the defaults depend on the source color space.)
//     */
//     jpeg_set_defaults(&cinfo);
//     /* Now you can set any non-default parameters you wish to.
//     * Here we just illustrate the use of quality (quantization table) scaling:
//     */
//     jpeg_set_quality(&cinfo, m_nQuality, true /* limit to baseline-JPEG values */);
//
//     cinfo.density_unit=1;
//     cinfo.X_density=0;// (unsigned short)GetXDPI();
//     cinfo.Y_density=0;//(unsigned short)GetYDPI();
//
//     /* Step 4: Start compressor */
//     /* true ensures that we will write a complete interchange-JPEG file.
//     * Pass true unless you are very sure of what you're doing.
//     */
//     jpeg_start_compress(&cinfo, true);
//
//     /* Step 5: while (scan lines remain to be written) */
//     /*           jpeg_write_scanlines(...); */
//     /* Here we use the library's state variable cinfo.next_scanline as the
//     * loop counter, so that we don't have to keep track ourselves.
//     * To keep things simple, we pass one scanline per call; you can pass
//     * more if you wish, though.
//     */
//     row_stride = m_lpImgData->GetRowWidthBytes();    /* JSAMPLEs per row in image_buffer */
//
//     //<DP> "8+row_stride" fix heap deallocation problem during debug???
//     buffer = (*cinfo.mem->alloc_sarray)
//         ((j_common_ptr) &cinfo, JPOOL_IMAGE, 8+row_stride, 1);
//
//     // CImageIterator iter(this);
//
//     // iter.upset();
//     int        y = cinfo.image_height - 1;
//     while (cinfo.next_scanline < cinfo.image_height) {
//         // info.nProgress = (long)(100*cinfo.next_scanline/cinfo.image_height);
//         uint8_t *pRow = m_lpImgData->GetImgRow(y);
//         if (m_lpImgData->GetPaletteItemCount() == 0)
//         {
//             m_lpImgData->RGBtoBGR(pRow, buffer[0], row_stride);
//             (void) jpeg_write_scanlines(&cinfo, buffer, 1);
//             // swap R & B for RGB images
//             // RGBtoBGR(buffer[0], row_stride);    // Lance : 1998/09/01 : Bug ID: EXP-2.1.1-9
//         }
//         else
//             (void) jpeg_write_scanlines(&cinfo, &pRow, 1);
//         y--;
//     }
//
//     /* Step 6: Finish compression */
//     jpeg_finish_compress(&cinfo);
//
//     /* Step 7: release JPEG compression object */
//     /* This is an important step since it will release a good deal of memory. */
//     jpeg_destroy_compress(&cinfo);
//
//     return true;
// }
