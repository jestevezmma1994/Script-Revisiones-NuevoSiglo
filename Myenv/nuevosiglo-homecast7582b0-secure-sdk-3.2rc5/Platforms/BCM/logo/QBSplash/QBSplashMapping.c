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

#include "QBSplashMapping.h"

#if defined(_CFE_)
#include "lib_types.h"
#elif !defined(__KERNEL__)
#include <stdint.h>
#include <stddef.h>
#endif // __KERNEL__
#include "QBSplashVDCInternal.h"
#include "QBSplashInternal.h"

#ifdef QB_SPLASH_MAPPING_TO_BUFFER
// this actually allocates an output buffer and copies data to it
void *QBSplashMappingToBuffer(QBSplashMapping *self, size_t *size) {
    void *buf;
    char *ptr;
    uint32_t *offsets;
    int bitmapCount = 0;
    int bitmapDataSize = 0;
    uint32_t bitmapStartId = 0;
    uint32_t i, j;
    QBSplashSerializedHeader serializedHeader;
    QBSplashVDCHeader *vdcHeader = (QBSplashVDCHeader *)self->vdcData;

    ASSERT_LOG(vdcHeader->outputProfile == self->outputProfile, NULL);

    *size = sizeof(QBSplashSerializedHeader); // main header size
    *size += self->entryCount * sizeof(QBSplashSerializedEntry); // entry headers size

    for (i = 0; i < self->entryCount; i++) {
        bitmapCount += self->entries[i].bitmapCount;
        for (j = 0; j < self->entries[i].bitmapCount; j++) {
            bitmapDataSize += self->entries[i].bitmaps[j].size;
        }
    }
    *size += bitmapCount * sizeof(uint32_t); // offsets size
    *size += bitmapCount * sizeof(QBSplashSerializedBitmapHeader); // bitmap headers size
    *size += bitmapDataSize; // bitmap data size
    *size += self->vdcSize;

    buf = QBBootMalloc(*size);
    if (!buf) {
        *size = 0;
        return NULL;
    }
    ptr = (char*)buf;

    // full serialized splash header
    serializedHeader.magic = QB_SPLASH_MAGIC;
    serializedHeader.version = QBSplashVersion_3;
    serializedHeader.outputProfile = self->outputProfile;
    serializedHeader.bgcolor = self->bgcolor;
    serializedHeader.entryCount = self->entryCount;
    serializedHeader.size = *size;
    serializedHeader.vdcSize = self->vdcSize;

    // place serialized splash header in output buffer
    memcpy(ptr, &serializedHeader, sizeof(QBSplashSerializedHeader));
    ptr += sizeof(QBSplashSerializedHeader);

    // place each entry in output buffer
    for (i = 0; i < self->entryCount; i++) {
        QBSplashSerializedEntry serializedEntry;
        serializedEntry.type = self->entries[i].type;
        serializedEntry.target = self->entries[i].target;
        serializedEntry.startId = bitmapStartId;
        serializedEntry.bitmapCount = self->entries[i].bitmapCount;

        memcpy(ptr, &serializedEntry, sizeof(QBSplashSerializedEntry));
        ptr += sizeof(QBSplashSerializedEntry);

        bitmapStartId += self->entries[i].bitmapCount;
    }

    // get pointer to the offset table
    offsets = (uint32_t *)ptr;
    ptr += bitmapCount * sizeof(uint32_t);

    // place each bitmap in the output buffer
    bitmapCount = 0;
    for (i = 0; i < self->entryCount; i++) {
        for (j = 0; j < self->entries[i].bitmapCount; j++) {
            QBSplashSerializedBitmapHeader serializedBitmapHeader;

            offsets[bitmapCount] = (uint32_t)(ptr - (char*)buf); // place offset to the current bitmap in the offset table

            serializedBitmapHeader.format = self->entries[i].bitmaps[j].format;
            serializedBitmapHeader.width = self->entries[i].bitmaps[j].width;
            serializedBitmapHeader.height = self->entries[i].bitmaps[j].height;
            serializedBitmapHeader.left = self->entries[i].bitmaps[j].left;
            serializedBitmapHeader.top = self->entries[i].bitmaps[j].top;
            serializedBitmapHeader.size = self->entries[i].bitmaps[j].size;

            memcpy(ptr, &serializedBitmapHeader, sizeof(QBSplashSerializedBitmapHeader));
            ptr += sizeof(QBSplashSerializedBitmapHeader);
            memcpy(ptr, self->entries[i].bitmaps[j].pixels, self->entries[i].bitmaps[j].size);
            ptr += self->entries[i].bitmaps[j].size;

            bitmapCount++;
        }
    }
    memcpy(ptr, self->vdcData, self->vdcSize);
    ptr += self->vdcSize;

    return buf;
}
#endif // QB_SPLASH_SERIALIZE

QBSplashMapping *QBSplashMappingCreateFromBuffer(void *inputBuffer, size_t bufferSize) {
    QBSplashSerializedHeader *serializedHeader;
    QBSplashSerializedEntry *serializedEntries;
    QBSplashSerializedBitmapHeader *serializedBitmap;
    QBSplashVDCHeader *vdcHeader;
    uint32_t *offsets;
    QBSplashMapping *self;
    QBSplashOutputDimensions outputDimensions;
    char *outputBufferPointer;
    int outputBufferSize = 0;
    uint32_t i, j;
    char *inputBufferPointer = 0;

    // get pointer serialized header
    ASSERT_LOG(inputBuffer != NULL, NULL);
    ASSERT_LOG(sizeof(QBSplashSerializedHeader) <= bufferSize, NULL);
    serializedHeader = inputBuffer;
    ASSERT_LOG(serializedHeader->size <= bufferSize, NULL);
    ASSERT_LOG(serializedHeader->version == QBSplashVersion_3, NULL);

    // get output dimensions
    outputDimensions = QBSplashGetOutputDimensions(serializedHeader->outputProfile);

    // get pointer to serialized entry list
    ASSERT_LOG(sizeof(QBSplashSerializedHeader) + serializedHeader->entryCount * sizeof(QBSplashSerializedEntry) <= bufferSize, NULL);
    serializedEntries = inputBuffer + sizeof(QBSplashSerializedHeader);

    // get pointer to the offsets
    offsets = inputBuffer + sizeof(QBSplashSerializedHeader) + serializedHeader->entryCount * sizeof(QBSplashSerializedEntry);

    // count the size needed for QBSplash_t, QBSplashEntry_t-s and QBSplashBitmap_t-s
    // increase size by QBSplash_t
    outputBufferSize = sizeof(QBSplashMapping);

    // increase size by the QBSplashEntry_t entries
    outputBufferSize += serializedHeader->entryCount * sizeof(QBSplashEntry);
    ASSERT_LOG(serializedHeader->vdcSize > 0, NULL);
    outputBufferSize += serializedHeader->vdcSize;

    // increase size by the QBSplashBitmap_t bitmaps
    for (i = 0; i < serializedHeader->entryCount; i++) {
        outputBufferSize += serializedEntries[i].bitmapCount * sizeof(QBSplashBitmap);
    }

    // check all bitmaps before allocating any memory
    for (i = 0; i < serializedHeader->entryCount; i++) {
        for (j = 0; j < serializedEntries[i].bitmapCount; j++) {
            ASSERT_LOG(
                    sizeof(QBSplashSerializedHeader)
                            + serializedHeader->entryCount
                                    * sizeof(QBSplashSerializedEntry)
                            + (serializedEntries[i].startId + j)
                                    * sizeof(uint32_t) <= bufferSize,
                    NULL);
            ASSERT_LOG(offsets[serializedEntries[i].startId + j] <= bufferSize, NULL);
            ASSERT_LOG(offsets[serializedEntries[i].startId + j] + sizeof(QBSplashSerializedBitmapHeader) <= bufferSize, NULL);

            serializedBitmap = inputBuffer + offsets[serializedEntries[i].startId + j];
#ifdef QBSPLASH_VERBOSE
            QBBootLog("offset: %d, width: %d, height: %d\n", offsets[serializedEntries[i].startId + j] + sizeof(QBSplashSerializedBitmapHeader), serializedBitmap->width, serializedBitmap->height);
#endif
            ASSERT_LOG(serializedBitmap->format == QBBitmapFormat_RGB565, NULL);
            ASSERT_LOG(serializedBitmap->width * serializedBitmap->height * sizeof(uint16_t) == serializedBitmap->size, NULL);
            if (serializedEntries[i].target == QBSplashOutputType_primary) {
                ASSERT_LOG(serializedBitmap->left + serializedBitmap->width <= outputDimensions.primaryWidth, NULL);
                ASSERT_LOG(serializedBitmap->top + serializedBitmap->height <= outputDimensions.primaryHeight, NULL);
            } else {
                ASSERT_LOG(serializedBitmap->left + serializedBitmap->width <= outputDimensions.secondaryWidth, NULL);
                ASSERT_LOG(serializedBitmap->top + serializedBitmap->height <= outputDimensions.secondaryHeight, NULL);
            }
        }
    }

    // allocate the output buffer
    self = QBBootMalloc(outputBufferSize);
    outputBufferPointer = (char *)self;

    // reserve space for QBSplash_t
    self->version = serializedHeader->version;
    self->outputProfile = serializedHeader->outputProfile;
    self->bgcolor = serializedHeader->bgcolor;
    self->entryCount = serializedHeader->entryCount;
    outputBufferPointer += sizeof(QBSplashMapping);

    // reserve space for QBSplashEntry_t entries
    self->entries = (QBSplashEntry *)outputBufferPointer;
    outputBufferPointer += self->entryCount * sizeof(QBSplashEntry);
    for (i = 0; i < self->entryCount; i++) {
        self->entries[i].type = serializedEntries[i].type;
        self->entries[i].target = serializedEntries[i].target;
        self->entries[i].bitmapCount = serializedEntries[i].bitmapCount;

        // reserve space for QBSplashBitmap_t bitmaps
        self->entries[i].bitmaps = (QBSplashBitmap *)outputBufferPointer;
        outputBufferPointer += self->entries[i].bitmapCount * sizeof(QBSplashBitmap);

        // the bitmaps were already checked earlier
        for (j = 0; j < self->entries[i].bitmapCount; j++) {
            serializedBitmap = inputBuffer + offsets[serializedEntries[i].startId + j];
            self->entries[i].bitmaps[j].format = serializedBitmap->format;
            self->entries[i].bitmaps[j].width = serializedBitmap->width;
            self->entries[i].bitmaps[j].height = serializedBitmap->height;
            self->entries[i].bitmaps[j].left = serializedBitmap->left;
            self->entries[i].bitmaps[j].top = serializedBitmap->top;
            self->entries[i].bitmaps[j].size = serializedBitmap->size;
            self->entries[i].bitmaps[j].pixels = inputBuffer + offsets[serializedEntries[i].startId + j] + sizeof(QBSplashSerializedBitmapHeader);
            inputBufferPointer = (char*)self->entries[i].bitmaps[j].pixels + self->entries[i].bitmaps[j].size; // point after the bitmap
        }
    }
    // copy VDC
    self->vdcData = outputBufferPointer;
    self->vdcSize = serializedHeader->vdcSize;
    memcpy(self->vdcData, inputBufferPointer, self->vdcSize);
    outputBufferPointer += self->vdcSize;

    vdcHeader = (QBSplashVDCHeader *)self->vdcData;
    ASSERT_LOG(vdcHeader->outputProfile == self->outputProfile, NULL);

    return self;
}

void QBSplashMappingDestroy(QBSplashMapping *self) {
    if (self) {
        QBBootFree(self);
    }
}
