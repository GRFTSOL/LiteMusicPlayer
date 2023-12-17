#include "RawImageData.h"
#include "ILIO.h"

extern "C" {
#include "../third-parties/libspng/spng/spng.h"
}


void preMultiplyRGBChannels(RawImageData *imgData) {
    assert(imgData->pixFormat == PF_RGBA32);

    // pre-multiply rgb channels with alpha channel
    auto row = imgData->rowPtr(0);
    for (int y = 0; y < imgData->height; y++) {
        auto p = row;
        for (int x = 0; x < imgData->width; x++) {
            auto a = p[3];
            p[0] = p[0] * a / 255;
            p[1] = p[1] * a / 255;
            p[2] = p[2] * a / 255;
            p += 4;
        }
        row += imgData->stride;
    }
}

int spngReadWriteFn(spng_ctx *ctx, void *user, void *dst_src, size_t length) {
    IILIO *io = (IILIO *)user;
    return io->read(dst_src, length) != length;
}

RawImageDataPtr loadRawImageDataFromPngFile(IILIO *io) {
    const size_t limit = 1024 * 1024 * 64;
    RawImageDataPtr imgData;
    spng_ctx *ctx = spng_ctx_new(0);

    if(ctx == NULL) {
        goto R_FAILED;
    }

    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_stream(ctx, spngReadWriteFn, io);

    struct spng_ihdr ihdr;
    int ret;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret) {
        goto R_FAILED;
    }

    // struct spng_plte plte = {0};
    // ret = spng_get_plte(ctx, &plte);
    // if (ret && ret != SPNG_ECHUNKAVAIL) {
    //     goto R_FAILED;
    // }

    int fmt;
    fmt = SPNG_FMT_RGB8;
    if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA || ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
        fmt = SPNG_FMT_RGBA8;
    }

    ret = spng_decode_image(ctx, NULL, 0, fmt, SPNG_DECODE_TRNS | SPNG_DECODE_GAMMA | SPNG_DECODE_PROGRESSIVE);
    if (ret) {
        goto R_FAILED;
    }

    imgData = make_shared<RawImageData>();
    if (!imgData->create(ihdr.width, ihdr.height, fmt == SPNG_FMT_RGBA8 ? 32 : 24)) {
        goto R_FAILED;
    }

    struct spng_row_info row_info;
    memset(&row_info, 0, sizeof(row_info));
    do {
        ret = spng_get_row_info(ctx, &row_info);
        if (ret) break;
        auto row = imgData->rowPtr(row_info.row_num);

        ret = spng_decode_row(ctx, row, imgData->stride);
    } while(!ret);

    if (fmt == SPNG_FMT_RGBA8) {
        preMultiplyRGBChannels(imgData.get());
    }

    spng_ctx_free(ctx);

    return imgData;

R_FAILED:
    spng_ctx_free(ctx);

    return nullptr;
}
