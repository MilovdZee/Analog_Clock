#include <LittleFS.h>
#include "BMPHelper.h"

boolean readBMPFile(char *fileName, BMPInfo *bmpInfo) {
  File file = LittleFS.open(fileName, "r");
  if (!file) return false;

  // find total file size
  int fileSize = file.size();

  // get image data offset
  uint16_t imageDataOffset;
  file.seek(0x0a);
  file.read((uint8_t *)&imageDataOffset, sizeof(imageDataOffset));

  // get image width
  file.seek(0x12);
  file.read((uint8_t *) & (bmpInfo->width), sizeof(bmpInfo->width));

  // get image height
  file.seek(0x16);
  file.read((uint8_t *) & (bmpInfo->height), sizeof(bmpInfo->height));

  // get bits per pixel
  file.seek(0x1c);
  file.read((uint8_t *) & (bmpInfo->bitsPerPixel), sizeof(bmpInfo->bitsPerPixel));

  // get DIB size
  uint16_t dibSize;
  file.seek(0x0e);
  file.read((uint8_t *) &dibSize, sizeof(dibSize));

  // read colortable (only supported for 1 bit/pixel images)
  if (bmpInfo->bitsPerPixel == 1) {
    for (int colorIndex = 0; colorIndex <= bmpInfo->bitsPerPixel; colorIndex++) {
      file.seek(0x0e + dibSize + colorIndex * 4);
      uint32_t colorTableEntry;
      file.read((uint8_t *)&colorTableEntry, 4);
      uint8_t red = *((uint8_t *)&colorTableEntry);
      uint8_t green = *((uint8_t *)&colorTableEntry + 1);
      uint8_t blue = *((uint8_t *)&colorTableEntry + 2);
      uint16_t color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
      bmpInfo->colorTable[colorIndex] = color;
    }
  }

  // allocate buffer
  bmpInfo->paddedWidth = bmpInfo->width % (32 / bmpInfo->bitsPerPixel) ? bmpInfo->width - bmpInfo->width % (32 / bmpInfo->bitsPerPixel) + 32 / bmpInfo->bitsPerPixel : bmpInfo->width;
  int bufferSize = bmpInfo->paddedWidth * bmpInfo->height * bmpInfo->bitsPerPixel / 8;

  Serial.printf("readBMPFile: fileName='%s', imageDataOffset=%d, width=%d, height=%d, bitsPerPixel=%d, dibSize=%d, paddedWidth=%d, bufferSize=%d\n", 
    fileName, imageDataOffset, bmpInfo->width, bmpInfo->height, bmpInfo->bitsPerPixel, dibSize, bmpInfo->paddedWidth, bufferSize);

  bmpInfo->pixelBuffer = (uint8_t *)malloc(bufferSize);
  if (bmpInfo->pixelBuffer == NULL) return false;

  // read the image data to the buffer
  file.seek(imageDataOffset);
  file.read(bmpInfo->pixelBuffer, bufferSize);

  return true;
}

uint16_t getPixelColor(BMPInfo *bmpInfo, uint8_t x, uint8_t y) {
  // Only support two modes: 1 bit or 16 bit per pixel
  if (bmpInfo -> bitsPerPixel == 1) {
    uint16_t bufferOffset = (x + y * bmpInfo->paddedWidth) / 8;
    uint16_t color = bmpInfo->pixelBuffer[bufferOffset] & 0x80 >> x % 8 ? bmpInfo->colorTable[1] : bmpInfo->colorTable[0];
    return color;
  } else if (bmpInfo -> bitsPerPixel == 16) {
    uint16_t bufferOffset = (x + y * bmpInfo->paddedWidth) * 2;
    uint16_t color = *(uint16_t *)(bmpInfo->pixelBuffer + bufferOffset);
    return color;
  } else {
    return 0;
  }
}
