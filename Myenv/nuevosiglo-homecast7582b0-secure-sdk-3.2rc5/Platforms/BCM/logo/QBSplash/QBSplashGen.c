/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp. z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QBSPLASH_DEBUG
// enable asserts and logging
#define QBSPLASH_DEBUG
#endif

#if !defined(PROFILE_NAME)
#error profile name not defined
#endif

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "QBSplash.h"
#include "QBSplashInternal.h"
#include "QBSplashMapping.h"
#include "QBSplashDefines.h"
#include "png.h"

typedef union {
    struct {
        float r;
        float g;
        float b;
        float a;
    } c;
    float channel[4];
} pixel_t;

#define BGCOLOR_R   0x00
#define BGCOLOR_G   0x00
#define BGCOLOR_B   0x00
#define BGCOLOR_A   0xff

#define __output_profile(x) QBSplashOutputProfile_ ## x
#define _output_profile(x) __output_profile(x)
#define OUTPUT_PROFILE _output_profile(PROFILE_NAME)

#define CLAMP(v, min, max) if (v < min) { v = min; } else if (v > max) { v = max; }

#define PIXEL16_TO_RGB(pixel, r, g, b) {\
    (b) = (((pixel) << 3) & 0xf8);\
    (g) = (((pixel) >> 3) & 0xfc);\
    (r) = (((pixel) >> 8) & 0xf8);}

#define PNGSIGSIZE 8

pixel_t RGBAToPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    pixel_t out = { r/255.0f, g/255.0f, b/255.0f, a/255.0f };
    return out;
}

void PixelToRGBA(pixel_t input, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a, double gamma) {
    *r = pow(input.c.r, gamma) * 255.0;
    *g = pow(input.c.g, gamma) * 255.0;
    *b = pow(input.c.b, gamma) * 255.0;
    *a = input.c.a * 255.0;
}

uint32_t PixelToUint32(pixel_t input, double gamma) {
    uint8_t r, g, b, a;

    PixelToRGBA(input, &r, &g, &b, &a, gamma);
    return RGBA_TO_PIXEL32(r, g, b, a);
}

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    png_voidp file = png_get_io_ptr(pngPtr);

    fread(data, length, 1, file);
}

pixel_t pixelGamma(pixel_t input, double gamma) {
    pixel_t out;
    out.c.r = pow(input.c.r, gamma);
    out.c.g = pow(input.c.g, gamma);
    out.c.b = pow(input.c.b, gamma);
    out.c.a = input.c.a;
    return out;
}

void imgRGBAToPixelGamma(pixel_t *dst, uint32_t *src, int w, int h, double gamma) {
    uint8_t r, g, b, a;
    pixel_t pixel;
    for (int i = 0; i < w * h; ++i) {
        PIXEL32_TO_RGBA(src[i], r, g, b, a);
        pixel_t pixel = {pow(r / 255.0, gamma), pow(g / 255.0, gamma), pow(b / 255.0, gamma), a / 255.0};
        dst[i] = pixel;
    }
}

void imgRGBAToPixel(pixel_t *dst, uint32_t *src, int w, int h) {
    uint8_t r, g, b, a;
    pixel_t pixel;
    for (int i = 0; i < w * h; ++i) {
        PIXEL32_TO_RGBA(src[i], r, g, b, a);
        pixel_t pixel = {r / 255.0, g / 255.0, b / 255.0, a / 255.0};
        dst[i] = pixel;
    }
}

void imgRGB565ToPixelGamma(pixel_t *dst, uint16_t *src, int w, int h, double gamma) {
    uint8_t r, g, b;
    pixel_t pixel;
    for (int i = 0; i < w * h; ++i) {
        PIXEL16_TO_RGB(src[i], r, g, b);
        pixel_t pixel = {pow(r / 255.0, gamma), pow(g / 255.0, gamma), pow(b / 255.0, gamma), 0.0};
        dst[i] = pixel;
    }
}

bool validatePng(FILE *file)
{
    png_byte pngSig[PNGSIGSIZE];

    int not_png = 0;

    if (!fread(pngSig, PNGSIGSIZE, 1, file)) {
        return false;
    }

    not_png = png_sig_cmp(pngSig, 0, PNGSIGSIZE);

    return (!not_png);
}

pixel_t *loadPng(const char *fileName, int *imgW, int *imgH)
{
    png_bytep* rowPtrs = NULL;
    uint32_t *pixelData = NULL;
    pixel_t *output = NULL;
    FILE * pngFile = fopen(fileName, "rb");
    double gamma = 2.2;

    png_structp pngPtr = NULL;
    png_infop infoPtr = NULL;

    if (!pngFile || !validatePng(pngFile)) {
        fprintf(stderr, "Cannot open file %s\n", fileName);
        goto out;
    }

    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr) {
        fprintf(stderr, "Couldn't initialize png read struct\n");
        goto out;
    }

    infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        fprintf(stderr, "Couldn't initialize png info sruct\n");
        png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
        goto out;
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        fprintf(stderr, "An error occured while reading the PNG file\n");
        png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
        goto out;
    }

    png_init_io(pngPtr, pngFile);

    png_set_sig_bytes(pngPtr, PNGSIGSIZE);

    png_read_info(pngPtr, infoPtr);
    *imgW = png_get_image_width(pngPtr, infoPtr);
    *imgH = png_get_image_height(pngPtr, infoPtr);

    uint32_t bitDepth = png_get_bit_depth(pngPtr, infoPtr);
    uint32_t channels = png_get_channels(pngPtr, infoPtr);
    uint32_t color_type = png_get_color_type(pngPtr, infoPtr);

    int intent;
    double imageGamma;
    if (png_get_gAMA(pngPtr, infoPtr, &imageGamma)) { // if the image has gamma other than 2.2, set it here
        gamma = imageGamma;
    }

    if (color_type != PNG_COLOR_TYPE_RGBA) {
        png_set_add_alpha(pngPtr, 0xff, PNG_FILLER_AFTER);
        channels += 1;
    }

    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(pngPtr);
        channels += 1;
    }

    if (bitDepth != 8) {
        fprintf(stderr, "PNG is not 8 bits per channel\n");
        goto out;
    }
    if (channels != 4) {
        fprintf(stderr, "PNG has %d channels instead of 4\n", channels);
        goto out;
    }

    rowPtrs = malloc((*imgH) * sizeof(png_bytep));

    const unsigned int stride = (*imgW) * bitDepth * channels / 8;
    pixelData = malloc((*imgH) * (*imgW) * sizeof(uint32_t));

    for (size_t i = 0; i < (*imgH); i++) {
        rowPtrs[i] = (png_bytep)pixelData + i * stride;
    }

    png_read_image(pngPtr, rowPtrs);

    // convert to pixel_t and adjust gamma at the same time
    output = malloc((*imgW) * (*imgH) * sizeof(pixel_t));
    imgRGBAToPixelGamma(output, pixelData, *imgW, *imgH, gamma);
    free(pixelData);

out:
    if (rowPtrs) {
        free(rowPtrs);
    }
    if (pngFile) {
        fclose(pngFile);
    }
    if (pngPtr || infoPtr) {
        png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
    }
    // can return NULL if file was not read correctly
    return output;
}

int savePng(const char *fileName, pixel_t *image, int imgW, int imgH) {
    int result = 0;
    FILE *pngFile = NULL;
    png_structp pngPtr = NULL;
    png_infop infoPtr = NULL;
    png_bytep rowPtr = NULL;
    double outputGamma = 1.0/2.2; // assume output gamma of 2.2

    pngFile = fopen(fileName, "wb");
    if (!pngFile) {
        fprintf(stderr, "Cannot open file %s for writing: \n", fileName);
        result = -1;
        goto out;
    }

    pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr) {
        fprintf(stderr, "Could not allocate write struct\n");
        result = -2;
        goto out;
    }

    infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        fprintf(stderr, "Could not allocate info struct\n");
        result = -2;
        goto out;
    }

    // libpng, srsly? :(
    if (setjmp(png_jmpbuf(pngPtr))) {
        fprintf(stderr, "Error during png creation\n");
        result = -3;
        goto out;
    }

    png_init_io(pngPtr, pngFile);

    png_set_IHDR(pngPtr, infoPtr, imgW, imgH, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_set_gAMA(pngPtr, infoPtr, outputGamma);
    png_write_info(pngPtr, infoPtr);

    rowPtr = (png_bytep)malloc(3 * imgW * sizeof(png_byte));
    uint32_t pixel = 0;
    uint32_t r = 0, g = 0, b = 0, a = 0;
    for (int y = 0; y < imgH; ++y) {
        for (int x = 0; x < imgW; ++x) {
            pixel = PixelToUint32(image[y * imgW + x], outputGamma);
            PIXEL32_TO_RGBA(pixel, r, g, b, a);
            rowPtr[x * 3 + 0] = r;
            rowPtr[x * 3 + 1] = g;
            rowPtr[x * 3 + 2] = b;
        }
        png_write_row(pngPtr, rowPtr);
    }
    png_write_end(pngPtr, NULL);

out:
    if (rowPtr) {
        free(rowPtr);
    }
    if (pngFile) {
        fclose(pngFile);
    }
    if (infoPtr) {
        png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
    }
    if (pngPtr) {
        png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
    }
    return result;
}

// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
// when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
// between B and C.  A and B are used to calculate slopes at the edges.
float cubicHermite(float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;

    return a*t*t*t + b*t*t + c*t + d;
}

pixel_t getPixelClamped(pixel_t *image, int x, int y, int width, int height)
{
    CLAMP(x, 0, width - 1);
    CLAMP(y, 0, height - 1);
    return image[y * width + x];
}

pixel_t sampleBicubic(pixel_t *image, float u, float v, int width, int height)
{
    // calculate coordinates -> also need to offset by half a pixel to keep image from shifting down and left half a pixel
    float x = (u * width) - 0.5;
    int xint = (int)x;
    float xfract = x - floor(x);

    float y = (v * height) - 0.5;
    int yint = (int)y;
    float yfract = y - floor(y);

    // 1st row
    pixel_t p00 = getPixelClamped(image, xint - 1, yint - 1, width, height);
    pixel_t p10 = getPixelClamped(image, xint + 0, yint - 1, width, height);
    pixel_t p20 = getPixelClamped(image, xint + 1, yint - 1, width, height);
    pixel_t p30 = getPixelClamped(image, xint + 2, yint - 1, width, height);

    // 2nd row
    pixel_t p01 = getPixelClamped(image, xint - 1, yint + 0, width, height);
    pixel_t p11 = getPixelClamped(image, xint + 0, yint + 0, width, height);
    pixel_t p21 = getPixelClamped(image, xint + 1, yint + 0, width, height);
    pixel_t p31 = getPixelClamped(image, xint + 2, yint + 0, width, height);

    // 3rd row
    pixel_t p02 = getPixelClamped(image, xint - 1, yint + 1, width, height);
    pixel_t p12 = getPixelClamped(image, xint + 0, yint + 1, width, height);
    pixel_t p22 = getPixelClamped(image, xint + 1, yint + 1, width, height);
    pixel_t p32 = getPixelClamped(image, xint + 2, yint + 1, width, height);

    // 4th row
    pixel_t p03 = getPixelClamped(image, xint - 1, yint + 2, width, height);
    pixel_t p13 = getPixelClamped(image, xint + 0, yint + 2, width, height);
    pixel_t p23 = getPixelClamped(image, xint + 1, yint + 2, width, height);
    pixel_t p33 = getPixelClamped(image, xint + 2, yint + 2, width, height);

    // interpolate bi-cubically!
    // Clamp the values since the curve can put the value below 0 or above 255
    pixel_t ret;
    for (int i = 0; i < 4; ++i)
    {
        float col0 = cubicHermite(p00.channel[i], p10.channel[i], p20.channel[i], p30.channel[i], xfract);
        float col1 = cubicHermite(p01.channel[i], p11.channel[i], p21.channel[i], p31.channel[i], xfract);
        float col2 = cubicHermite(p02.channel[i], p12.channel[i], p22.channel[i], p32.channel[i], xfract);
        float col3 = cubicHermite(p03.channel[i], p13.channel[i], p23.channel[i], p33.channel[i], xfract);
        float value = cubicHermite(col0, col1, col2, col3, yfract);
        CLAMP(value, 0.0f, 1.0f);
        ret.channel[i] = value;
    }
    return ret;
}

void resizeImage(pixel_t *dst, pixel_t *src, int dstW, int dstH, int srcW, int srcH)
{
    uint8_t *row = (uint8_t *)dst;
    int dstPitch = dstW * sizeof(pixel_t);
    for (int y = 0; y < dstH; ++y)
    {
        pixel_t *destPixel = (pixel_t *)row;
        float v = (float)y / (float)(dstH - 1);
        for (int x = 0; x < dstW; ++x)
        {
            float u = (float)x / (float)(dstW - 1);
            *destPixel = sampleBicubic(src, u, v, srcW, srcH);
            destPixel ++;
        }
        row += dstPitch;
    }
}

void blit(pixel_t *dst, int dstPitch, int dstLeft, int dstTop,
          pixel_t *src, int srcPitch, int srcLeft, int srcTop, int srcW, int srcH) {

    char *dstRow = (char*)dst + dstPitch * dstTop;
    char *srcRow = (char*)src + srcPitch * srcTop;
    for (int y = 0; y < srcH; y++) {
        pixel_t *dstPixel = (pixel_t *)dstRow + dstLeft;
        pixel_t *srcPixel = (pixel_t *)srcRow + srcLeft;
        for (int x = 0; x < srcW; x++) {
            dstPixel->c.r = srcPixel->c.r * srcPixel->c.a + dstPixel->c.r * (1.0f - srcPixel->c.a);
            dstPixel->c.g = srcPixel->c.g * srcPixel->c.a + dstPixel->c.g * (1.0f - srcPixel->c.a);
            dstPixel->c.b = srcPixel->c.b * srcPixel->c.a + dstPixel->c.b * (1.0f - srcPixel->c.a);
            dstPixel->c.a = 1.0f;
            srcPixel++;
            dstPixel++;
        }
        dstRow += dstPitch;
        srcRow += srcPitch;
    }
}

void drawPixel(pixel_t *img, int w, int h, int x, int y, pixel_t color) {
    if (x > 0 && x < w && y > 0 && y < h) {
        pixel_t dstPixel = img[y * w + x];
        dstPixel.c.r = color.c.r * color.c.a + dstPixel.c.r * (1.0f - color.c.a);
        dstPixel.c.g = color.c.g * color.c.a + dstPixel.c.g * (1.0f - color.c.a);
        dstPixel.c.b = color.c.b * color.c.a + dstPixel.c.b * (1.0f - color.c.a);
        dstPixel.c.a = 1.0f;
        img[y * w + x] = dstPixel;
    }
}

void drawRuler(pixel_t *img, int w, int h, int x1, int y1, int x2, int y2, pixel_t color1, pixel_t color2) {
    int count = 0;
    if (x1 == x2) { // vertical
        for (int y = y1; y < y2; y++) {
            if (count < 4) {
                drawPixel(img, w, h, x1, y, color1);
            } else {
                drawPixel(img, w, h, x1, y, color2);
            }
            count = (count + 1) % 8;
        }
    } else if (y1 == y2) { // horizontal
        for (int x = x1; x < x2; x++) {
            if (count < 4) {
                drawPixel(img, w, h, x, y1, color1);
            } else {
                drawPixel(img, w, h, x, y1, color2);
            }
            count = (count + 1) % 8;
        }
    }
}

// returns entry size
QBSplashBitmap prepareImage16(pixel_t *composeBuffer, int srcPitch, int srcLeft, int srcTop, int dstW, int dstH, QBSplashOutputType target) {
    QBSplashBitmap bitmap;
    double gamma;

    if (target == QBSplashOutputType_primary) {
        gamma = 1.0/PRIMARY_GAMMA;
    } else if (target == QBSplashOutputType_secondary) {
        gamma = 1.0/SECONDARY_GAMMA;
    } else {
        gamma = 1.0;
    }

    bitmap.pixels = malloc(dstW * dstH * sizeof(uint16_t));

    int dstPitch = dstW * sizeof(uint16_t);

    uint8_t r, g, b, a;

    char* dstRow = (char*)bitmap.pixels;
    char* srcRow = (char*)composeBuffer;
    for (int y = 0; y < dstH; y++) {
        pixel_t *srcPixel = (pixel_t *)srcRow;
        uint16_t *dstPixel = (uint16_t *)dstRow;
        for (int x = 0; x < dstW; x++) {
            PixelToRGBA(*srcPixel, &r, &g, &b, &a, gamma);
            *dstPixel = RGB_TO_PIXEL16(r, g, b);
            srcPixel++;
            dstPixel++;
        }
        dstRow += dstPitch;
        srcRow += srcPitch;
    }

    bitmap.format = QBBitmapFormat_RGB565;
    bitmap.width = dstW;
    bitmap.height = dstH;
    bitmap.size = dstW * dstH * sizeof(uint16_t);
    bitmap.left = srcLeft;
    bitmap.top = srcTop;

    return bitmap;
}

void dumpPixels(pixel_t *buf, const char *name, int width, int height, int id1, int id2, int id3) {
    uint32_t *tmp = malloc(width * height * sizeof(uint32_t));
    for (int i = 0; i < width * height; ++i) {
        tmp[i] = PixelToUint32(buf[i], 1.0 / 2.2);
    }
    char fname[100];
    snprintf(fname, 100, "%s_%d_%d_%d.data", name, id1, id2, id3);
    FILE * dbg = fopen(fname, "w");
    fwrite(tmp, width * height * sizeof(uint32_t), 1, dbg);
    fclose(dbg);
    free(tmp);
}

int addEntry(QBSplashMapping *splash, entry *e, QBSplashOutputType target, pixel_t **layerBuffers, int layer, char *imagePath) {
    char fname[100];
    pixel_t black = {0.0f, 0.0f, 0.0f, 0.0f};

    splash->entries[splash->entryCount].type = e->type;
    splash->entries[splash->entryCount].target = target;
    splash->entries[splash->entryCount].bitmapCount = e->count;
    splash->entries[splash->entryCount].bitmaps = malloc(e->count * sizeof(QBSplashBitmap));

    QBSplashBitmap *bitmaps = splash->entries[splash->entryCount].bitmaps;

    for (int i = 0; i < e->count; ++i) {
        if (layer > 0) {
            for (int k = 0; k < PRIMARY_WIDTH * PRIMARY_HEIGHT; k++) {
                layerBuffers[layer][k] = layerBuffers[layer - 1][k];
            }
        } else {
            for (int k = 0; k < PRIMARY_WIDTH * PRIMARY_HEIGHT; k++) {
                layerBuffers[layer][k] = RGBAToPixel(BGCOLOR_R, BGCOLOR_G, BGCOLOR_B, BGCOLOR_A);
            }
        }

        snprintf(fname, 100, "%s/%s", imagePath, e->fileNames[i]);

        int imgW, imgH;
        pixel_t *image = loadPng(fname, &imgW, &imgH);
        if (!image) {
            bitmaps[i].format = QBBitmapFormat_invalid;
            continue;
        }

        int left = e->xPos - imgW / 2;
        int top = e->yPos - imgH / 2;

        blit(layerBuffers[layer], PRIMARY_WIDTH * sizeof(pixel_t), left, top, image, imgW * sizeof(pixel_t), 0, 0, imgW, imgH);

        pixel_t *outBuffer = malloc(imgW * imgH * sizeof(pixel_t));
        for (int i = 0; i < imgW * imgH; ++i) {
            outBuffer[i] = black;
        }
        blit(outBuffer, imgW * sizeof(pixel_t), 0, 0, layerBuffers[layer], PRIMARY_WIDTH * sizeof(pixel_t), left, top, imgW, imgH);

        if (target == QBSplashOutputType_secondary) {
            int scaledW = imgW * ((float)SECONDARY_WIDTH / (float)PRIMARY_WIDTH);
            int scaledH = imgH * ((float)SECONDARY_HEIGHT / (float)PRIMARY_HEIGHT);
            int scaledLeft = left * ((float)SECONDARY_WIDTH / (float)PRIMARY_WIDTH);
            int scaledTop = top * ((float)SECONDARY_HEIGHT / (float)PRIMARY_HEIGHT);
            pixel_t *tmpBuffer = malloc(scaledW * scaledH * sizeof(pixel_t));
            resizeImage(tmpBuffer, outBuffer, scaledW, scaledH, imgW, imgH);
            bitmaps[i] = prepareImage16(tmpBuffer, scaledW * sizeof(pixel_t), scaledLeft, scaledTop, scaledW, scaledH, target);
            free(tmpBuffer);
        } else {
            bitmaps[i] = prepareImage16(outBuffer, imgW * sizeof(pixel_t), left, top, imgW, imgH, target);
        }
        free(outBuffer);
        free(image);
    }

    splash->entryCount++;

    return e->count;
}

int main(int argc, char * argv[]) {
    QBSplashMapping splash;
    QBSplashEntry splashEntry;

    if (argc != 4) {
        fprintf(stderr, "Usage: QBSplashGen <images_dir> <vdc_profile> <output_file>\n");
        exit(1);
    }

    char *imagePath = argv[1];
    char *vdcProfileName = argv[2];
    char *outputFileName = argv[3];
    pixel_t *layerBuffers[ENTRY_COUNT]; // HD buffer

    for (int i = 0; i < ENTRY_COUNT; i++) {
        layerBuffers[i] = malloc(PRIMARY_WIDTH * PRIMARY_HEIGHT * sizeof(pixel_t));
    }

    for (int i = 0; i < PRIMARY_WIDTH * PRIMARY_HEIGHT; i++) {
        layerBuffers[0][i] = RGBAToPixel(BGCOLOR_R, BGCOLOR_G, BGCOLOR_B, BGCOLOR_A);
    }

    splash.outputProfile = OUTPUT_PROFILE;
    splash.bgcolor = RGBA_TO_PIXEL32(BGCOLOR_R, BGCOLOR_G, BGCOLOR_B, BGCOLOR_A);
    splash.version = QBSplashVersion_3;
    splash.entryCount = 0;
    bool dualTarget = (splash.outputProfile == QBSplashOutputProfile_720p50_PAL || splash.outputProfile == QBSplashOutputProfile_720p60_NTSC);
    if (dualTarget) {
        splash.entries = malloc(ENTRY_COUNT * 2 * sizeof(QBSplashEntry));
    } else {
        splash.entries = malloc(ENTRY_COUNT * sizeof(QBSplashEntry));
    }

    uint32_t bitmapCount = 0;
    for (int i = 0; i < ENTRY_COUNT; i++) {
        bitmapCount += addEntry(&splash, entries[i], QBSplashOutputType_primary, layerBuffers, i, imagePath);
        if (dualTarget) {
            bitmapCount += addEntry(&splash, entries[i], QBSplashOutputType_secondary, layerBuffers, i, imagePath);
        }
    }

    FILE *vdcFile = fopen(vdcProfileName, "rb");
    if (!vdcFile) {
        fprintf(stderr, "Cannot open %s\n", vdcProfileName);
        return 1;
    }
    size_t profileSize;
    fseek(vdcFile, 0, SEEK_END);
    profileSize = ftell(vdcFile);
    fseek(vdcFile, 0, SEEK_SET);

    splash.vdcSize = profileSize;
    splash.vdcData = malloc(profileSize);
    fread(splash.vdcData, profileSize, 1, vdcFile);
    fclose(vdcFile);

    size_t size;
    void *buf = QBSplashMappingToBuffer(&splash, &size);
    if (!buf) {
        return 1;
    }
    FILE *outFile = fopen(outputFileName, "wb");
    if (!outFile) {
        fprintf(stderr, "Cannot open %s\n", outputFileName);
        return 1;
    }
    fwrite(buf, size, 1, outFile);
    fclose(outFile);

    QBSplashMapping *splash2 = QBSplashMappingCreateFromBuffer(buf, size);
    for (int i = 0; i < splash2->entryCount; i++) {
        printf("entry %d type %u outputType %u\n", i, (unsigned int)splash2->entries[i].type, (unsigned int)splash2->entries[i].target);
    }

    // check if splash was created properly
    ASSERT_LOG(splash.version == splash2->version, 0);
    ASSERT_LOG(splash.bgcolor == splash2->bgcolor, 0);
    ASSERT_LOG(splash.entryCount == splash2->entryCount, 0);
    ASSERT_LOG(splash.outputProfile == splash2->outputProfile, 0);
    for (int i = 0; i < splash.entryCount; i++) {
        ASSERT_LOG(splash.entries[i].bitmapCount == splash2->entries[i].bitmapCount, 0);
        ASSERT_LOG(splash.entries[i].type == splash2->entries[i].type, 0);
        for (int j = 0; j < splash.entries[i].bitmapCount; j++) {
            ASSERT_LOG(splash.entries[i].bitmaps[j].format == splash2->entries[i].bitmaps[j].format, 0);
            ASSERT_LOG(splash.entries[i].bitmaps[j].width == splash2->entries[i].bitmaps[j].width, 0);
            ASSERT_LOG(splash.entries[i].bitmaps[j].height == splash2->entries[i].bitmaps[j].height, 0);
            ASSERT_LOG(splash.entries[i].bitmaps[j].left == splash2->entries[i].bitmaps[j].left, 0);
            ASSERT_LOG(splash.entries[i].bitmaps[j].top == splash2->entries[i].bitmaps[j].top, 0);
            ASSERT_LOG(splash.entries[i].bitmaps[j].size == splash2->entries[i].bitmaps[j].size, 0);
            ASSERT_LOG(memcmp(splash.entries[i].bitmaps[j].pixels, splash2->entries[i].bitmaps[j].pixels, splash.entries[i].bitmaps[j].size) == 0, 0);
        }
    }

    // generate previews
    uint16_t *primarySurface = (uint16_t *) malloc(PRIMARY_WIDTH * PRIMARY_HEIGHT * sizeof(uint16_t));
    uint16_t *secondarySurface = (uint16_t *) malloc(SECONDARY_WIDTH * SECONDARY_HEIGHT * sizeof(uint16_t));
    QBSplash splashPreview = QBSplashGetInstance();

    QBSplashInit(splashPreview, buf, size);
    QBSplashAddSurface(splashPreview, primarySurface, QBBitmapFormat_RGB565, PRIMARY_WIDTH, PRIMARY_HEIGHT, PRIMARY_WIDTH * sizeof(uint16_t), QBSplashOutputType_primary);
    QBSplashAddSurface(splashPreview, secondarySurface, QBBitmapFormat_RGB565, SECONDARY_WIDTH, SECONDARY_HEIGHT, SECONDARY_WIDTH * sizeof(uint16_t), QBSplashOutputType_secondary);
    QBSplashDisplayLogo(splashPreview);
    QBSplashAnimationDraw(splashPreview);
    QBSplashDrawProgress(splashPreview, 50);

    pixel_t *primaryPreview = (pixel_t *) malloc(PRIMARY_WIDTH * PRIMARY_HEIGHT * sizeof(pixel_t));
    imgRGB565ToPixelGamma(primaryPreview, primarySurface, PRIMARY_WIDTH, PRIMARY_HEIGHT, PRIMARY_GAMMA);

    pixel_t *secondaryPreview = (pixel_t *) malloc(SECONDARY_WIDTH * SECONDARY_HEIGHT * sizeof(pixel_t));
    imgRGB565ToPixelGamma(secondaryPreview, secondarySurface, SECONDARY_WIDTH, SECONDARY_HEIGHT, SECONDARY_GAMMA);

    savePng("preview_primary.png", primaryPreview, PRIMARY_WIDTH, PRIMARY_HEIGHT);
    savePng("preview_secondary.png", secondaryPreview, SECONDARY_WIDTH, SECONDARY_HEIGHT);
    for (int i = 0; i < splash.entryCount; i++) {
        pixel_t *target = NULL;
        int previewWidth = 0;
        int previewHeight = 0;
        if (splash.entries[i].target == QBSplashOutputType_primary) {
            target = primaryPreview;
            previewWidth = PRIMARY_WIDTH;
            previewHeight = PRIMARY_HEIGHT;
        } else {
            target = secondaryPreview;
            previewWidth = SECONDARY_WIDTH;
            previewHeight = SECONDARY_HEIGHT;
        }
        int top = splash.entries[i].bitmaps[0].top;
        int left = splash.entries[i].bitmaps[0].left;
        int width = splash.entries[i].bitmaps[0].width;
        int height = splash.entries[i].bitmaps[0].height;
        pixel_t color1 = RGBAToPixel(0xff, 0x20, 0x00, 0xff);
        pixel_t color2 = RGBAToPixel(0x00, 0x20, 0xff, 0xff);
        // top
        drawRuler(target, previewWidth, previewHeight, 0, top, previewWidth - 1, top, color1, color2);
        // bottom
        drawRuler(target, previewWidth, previewHeight, 0, top + height - 1, previewWidth - 1, top + height - 1, color1, color2);
        // left
        drawRuler(target, previewWidth, previewHeight, left, 0, left, previewHeight - 1, color1, color2);
        // right
        drawRuler(target, previewWidth, previewHeight, left + width - 1, 0, left + width - 1, previewHeight - 1, color1, color2);
    }
    savePng("layout_primary.png", primaryPreview, PRIMARY_WIDTH, PRIMARY_HEIGHT);
    savePng("layout_secondary.png", secondaryPreview, SECONDARY_WIDTH, SECONDARY_HEIGHT);

    QBSplashDeInit(splashPreview);

    free(primarySurface);
    free(primaryPreview);
    free(secondarySurface);
    free(secondaryPreview);

    for (int i = 0; i < ENTRY_COUNT; i++) {
        free(layerBuffers[i]);
    }

    for (int i = 0; i < ENTRY_COUNT; i++) {
        free(splash.entries[i].bitmaps);
    }

    free(splash.entries);

    return 0;
}
