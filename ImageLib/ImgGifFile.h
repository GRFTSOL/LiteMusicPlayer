// ImgGifFile.h: interface for the CImgGifFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGGIFFILE_H__71E1ECB6_FCA2_490F_9623_22DC5572DB6F__INCLUDED_)
#define AFX_IMGGIFFILE_H__71E1ECB6_FCA2_490F_9623_22DC5572DB6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageIterator.h"

class GIF_RAW_IMAGE_DATA;

typedef short int       code_int;   

/* Various error codes used by decoder */
#define OUT_OF_MEMORY -10
#define BAD_CODE_SIZE -20
#define READ_ERROR -1
#define WRITE_ERROR -2
#define OPEN_ERROR -3
#define CREATE_ERROR -4
#define MAX_CODES   4095
#define GIFBUFTAM 16384
#define TRANSPARENCY_CODE 0xF9

//LZW GIF Image compression
#define MAXBITSCODES    12
#define HSIZE  5003     /* 80% occupancy */
#define MAXCODE(n_bits) (((code_int) 1 << (n_bits)) - 1)
#define HashTabOf(i)    htab[i]
#define CodeTabOf(i)    codetab[i]

struct rgb_color { uint8_t r,g,b; };

class CImgGifFile
{
public:
    virtual bool open(IILIO *io);
    // virtual bool save(IILIO *io);

public:
#pragma pack(push)
#pragma pack(1)

typedef struct tag_gifgce{
  uint8_t transpcolflag:1;
  uint8_t userinputflag:1;
  uint8_t dispmeth:3;
  uint8_t res:3;
  uint16_t delaytime;
  uint8_t transpcolindex;
} struct_gifgce;

typedef struct tag_dscgif{        /* Logic Screen Descriptor  */
  char header[6];                /* Firma and version */
  uint16_t scrwidth;
  uint16_t scrheight;
  char pflds;
  char bcindx;
  char pxasrat;
} struct_dscgif;

typedef struct tag_image{      /* Image Descriptor */
  uint16_t l;
  uint16_t t;
  uint16_t w;
  uint16_t h;
  uint8_t   pf;
} struct_image;

typedef struct tag_TabCol{        /* Tabla de colores */
  short colres;                    /* color resolution */
  short sogct;                    /* size of global color table */
  rgb_color paleta[256];        /* paleta */
} struct_TabCol;

typedef struct tag_RLE{
    int rl_pixel;
    int rl_basecode;
    int rl_count;
    int rl_table_pixel;
    int rl_table_max;
    int just_cleared;
    int out_bits;
    int out_bits_init;
    int out_count;
    int out_bump;
    int out_bump_init;
    int out_clear;
    int out_clear_init;
    int max_ocodes;
    int code_clear;
    int code_eof;
    unsigned int obuf;
    int obits;
    unsigned char oblock[256];
    int oblen;
} struct_RLE;
#pragma pack(pop)

    void setLoops(int loops);
    long getLoops();
    void setComment(const char* sz_comment_in);
    void getComment(char* sz_comment_out);
    void setDisposalMethod(int dm);
    long getDisposalMethod();

protected:
    bool decodeExtension(IILIO *io);
    void encodeHeader(IILIO *io);
    void encodeLoopExtension(IILIO *io);
    void encodeExtension(IILIO *io);
    void encodeBody(IILIO *io, bool bLocalColorMap = false);
    void encodeComment(IILIO *io);
    bool encodeRGB(IILIO *io);
    // void gifMix(CxImage & imgsrc2, long lxOffset, long lyOffset);
    
    struct_gifgce gifgce;

    int             curx, cury;
    long             CountDown;
    unsigned long    cur_accum;
    int              cur_bits;
    int interlaced, iypos, istep, iheight, ipass;
    int ibf;
    int ibfmax;
    uint8_t buf[GIFBUFTAM + 1];
// Implementation
    int GifNextPixel ();
    void Putword (int w, IILIO *io);
    void compressNONE (int init_bits, IILIO *io);
    void compressLZW (int init_bits, IILIO *io);
    void output (code_int code );
    void cl_hash (long hsize);
    void char_out (int c);
    void flush_char ();
    short init_exp(short size);
    short get_next_code(IILIO *io);
    short decoder(IILIO *io, CImageIterator* iter, short linewidth, int &bad_code_count);
    int get_byte(IILIO *io);
    int out_line(CImageIterator *iter, unsigned char *pixels, int linelen);
    int get_num_frames(IILIO *io,struct_TabCol* TabColSrc);

    short curr_size;                     /* The current code size */
    short clear;                         /* Value for a clear code */
    short ending;                        /* Value for a ending code */
    short newcodes;                      /* First available code */
    short top_slot;                      /* Highest code for current size */
    short slot;                          /* Last read code */

    /* The following static variables are used
    * for seperating out codes */
    short navail_bytes;              /* # bytes left in block */
    short nbits_left;                /* # bits left in current uint8_t */
    uint8_t b1;                           /* Current uint8_t */
    uint8_t byte_buff[257];               /* Current block */
    uint8_t *pbytes;                      /* Pointer to next uint8_t in block */
    /* The reason we have these seperated like this instead of using
    * a structure like the original Wilhite code did, is because this
    * stuff generally produces significantly faster code when compiled...
    * This code is full of similar speedups...  (For a good book on writing
    * C for speed or for space optomisation, see Efficient C by Tom Plum,
    * published by Plum-Hall Associates...)
    */
    uint8_t stack[MAX_CODES + 1];            /* Stack for storing pixels */
    uint8_t suffix[MAX_CODES + 1];           /* Suffix table */
    uint16_t prefix[MAX_CODES + 1];           /* Prefix linked list */

//LZW GIF Image compression routines
    long htab [HSIZE];
    unsigned short codetab [HSIZE];
    int n_bits;                /* number of bits/code */
    code_int maxcode;        /* maximum code, given n_bits */
    code_int free_ent;        /* first unused entry */
    int clear_flg;
    int g_init_bits;
    IILIO *g_ioOut;
    int ClearCode;
    int EOFCode;

    int a_count;
    char accum[256];

    char m_comment[256];
    int m_loops;
    int m_dispmeth;

//RLE compression routines
    void compressRLE( int init_bits, IILIO *io);
    void rle_clear(struct_RLE* rle);
    void rle_flush(struct_RLE* rle);
    void rle_flush_withtable(int count, struct_RLE* rle);
    void rle_flush_clearorrep(int count, struct_RLE* rle);
    void rle_flush_fromclear(int count,struct_RLE* rle);
    void rle_output_plain(int c,struct_RLE* rle);
    void rle_reset_out_clear(struct_RLE* rle);
    unsigned int rle_compute_triangle_count(unsigned int count, unsigned int nrepcodes);
    unsigned int rle_isqrt(unsigned int x);
    void rle_write_block(struct_RLE* rle);
    void rle_block_out(unsigned char c, struct_RLE* rle);
    void rle_block_flush(struct_RLE* rle);
    void rle_output(int val, struct_RLE* rle);
    void rle_output_flush(struct_RLE* rle);

public:
    void setImgData(GIF_RAW_IMAGE_DATA *imgData) { m_imgData = imgData; }
    GIF_RAW_IMAGE_DATA        *m_imgData;

};

#endif // !defined(AFX_IMGGIFFILE_H__71E1ECB6_FCA2_490F_9623_22DC5572DB6F__INCLUDED_)
