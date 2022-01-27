// ImgJpegFile.h: interface for the CImgJpegFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGJPEGFILE_H__F94DDA50_E905_4C0E_A2B8_C96AB2E0C835__INCLUDED_)
#define AFX_IMGJPEGFILE_H__F94DDA50_E905_4C0E_A2B8_C96AB2E0C835__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define CXIMAGEJPG_SUPPORT_EXIF 0

extern "C" {
 #include "./jpeg/jpeglib.h"
 #include "./jpeg/jerror.h"
};

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////        E X I F             //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
#if CXIMAGEJPG_SUPPORT_EXIF

#define MAX_COMMENT 1000

typedef struct tag_ExifInfo {
    char  CameraMake   [32];
    char  CameraModel  [40];
    char  DateTime     [20];
    int   Height, Width;
    int   IsColor;
    int   FlashUsed;
    float FocalLength;
    float ExposureTime;
    float ApertureFNumber;
    float Distance;
    float CCDWidth;
    float ExposureBias;
    int   Whitebalance;
    int   MeteringMode;
    int   ExposureProgram;
    int   ISOequivalent;
    int   CompressionLevel;
    char  Comments[MAX_COMMENT];

    unsigned char * ThumbnailPointer;  /* Pointer at the thumbnail */
    unsigned ThumbnailSize;     /* Size of thumbnail. */
} EXIFINFO;

class CxExifInfo
{

typedef struct tag_ExifTable{
    unsigned short Tag;
    char * Desc;
} EXIFTABLE;

public:
    EXIFINFO* m_exifinfo;
    char m_szLastError[256];
    CxExifInfo(EXIFINFO* info);
    bool decodeInfo(struct jpeg_decompress_struct cinfo);
protected:
    bool exif_process(unsigned char * CharBuf, unsigned int length, EXIFINFO* pInfo);
    int get16u(void * Short);
    int get32s(void * Long);
    unsigned long Get32u(void * Long);
    bool processExifDir(unsigned char * DirStart, unsigned char * OffsetBase, unsigned ExifLength,
                           EXIFINFO * const pInfo, unsigned char ** const LastExifRefdP);
    double convertAnyFormat(void * ValuePtr, int Format);
    bool is_exif(struct jpeg_marker_struct const marker);

    double FocalplaneXRes;
    double FocalplaneUnits;
    int ExifImageWidth;
    int MotorolaOrder;
};

    EXIFINFO m_exifinfo;

#endif //CXIMAGEJPG_SUPPORT_EXIF

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////        C x F i l e J p g         ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

// thanks to Chris Shearer Cooper <cscooper@frii.com>
class CxFileJpg : public jpeg_destination_mgr, public jpeg_source_mgr
    {
public:
    enum { eBufSize = 4096 };

    CxFileJpg(IILIO* io)
    {
        m_io = io;

        init_destination = initDestination;
        empty_output_buffer = emptyOutputBuffer;
        term_destination = termDestination;

        init_source = initSource;
        fill_input_buffer = fillInputBuffer;
        skip_input_data = skipInputData;
        resync_to_restart = jpeg_resync_to_restart; // use default method
        term_source = termSource;
        next_input_byte = nullptr; //* => next uint8_t to read from buffer 
        bytes_in_buffer = 0;    //* # of bytes remaining in buffer 

        m_pBuffer = new unsigned char[eBufSize];
    }
    ~CxFileJpg()
    {
        delete [] m_pBuffer;
    }

    static void initDestination(j_compress_ptr cinfo)
    {
        CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
        pDest->next_output_byte = pDest->m_pBuffer;
        pDest->free_in_buffer = eBufSize;
    }

    static boolean emptyOutputBuffer(j_compress_ptr cinfo)
    {
        CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
        if (pDest->m_io->write(pDest->m_pBuffer, eBufSize) != (size_t)eBufSize)
            ERREXIT(cinfo, JERR_FILE_WRITE);
        pDest->next_output_byte = pDest->m_pBuffer;
        pDest->free_in_buffer = eBufSize;
        return true;
    }

    static void termDestination(j_compress_ptr cinfo)
    {
        CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
        size_t datacount = eBufSize - pDest->free_in_buffer;
        /* write any data remaining in the buffer */
        if (datacount > 0) {
            if (pDest->m_io->write(pDest->m_pBuffer, datacount) != datacount)
                ERREXIT(cinfo, JERR_FILE_WRITE);
        }
        return;
    }

    static void initSource(j_decompress_ptr cinfo)
    {
        CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
        pSource->m_bStartOfFile = true;
    }

    static boolean fillInputBuffer(j_decompress_ptr cinfo)
    {
        size_t nbytes;
        CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
        nbytes = pSource->m_io->read(pSource->m_pBuffer, eBufSize);
        if (nbytes <= 0){
            if (pSource->m_bStartOfFile)    //* Treat empty input file as fatal error 
                ERREXIT(cinfo, JERR_INPUT_EMPTY);
            WARNMS(cinfo, JWRN_JPEG_EOF);
            // Insert a fake EOI marker 
            pSource->m_pBuffer[0] = (JOCTET) 0xFF;
            pSource->m_pBuffer[1] = (JOCTET) JPEG_EOI;
            nbytes = 2;
        }
        pSource->next_input_byte = pSource->m_pBuffer;
        pSource->bytes_in_buffer = nbytes;
        pSource->m_bStartOfFile = false;
        return true;
    }

    static void skipInputData(j_decompress_ptr cinfo, long num_bytes)
    {
        CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
        if (num_bytes > 0){
            while (num_bytes > (long)pSource->bytes_in_buffer){
                num_bytes -= (long)pSource->bytes_in_buffer;
                fillInputBuffer(cinfo);
                // note we assume that fill_input_buffer will never return false,
                // so suspension need not be handled.
            }
            pSource->next_input_byte += (size_t) num_bytes;
            pSource->bytes_in_buffer -= (size_t) num_bytes;
        }
    }

    static void termSource(j_decompress_ptr cinfo)
    {
        return;
    }
protected:
    IILIO        *m_io;
    unsigned char *m_pBuffer;
    bool m_bStartOfFile;
};

#endif // !defined(AFX_IMGJPEGFILE_H__F94DDA50_E905_4C0E_A2B8_C96AB2E0C835__INCLUDED_)
