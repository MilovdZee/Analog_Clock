// File MUST be SCREEN_DIAMETER x SCREEN_DIAMETER pixels in 565 format uncompressed

#include "BMPHelper.h"

void drawBMPFromFile(int xOffset, int yOffset, int rotationX, int rotationY, int rotation, boolean erase, char *fileName) {
  BMPInfo bmpInfo;
  if (readBMPFile(fileName, &bmpInfo)) {
    Serial.printf("drawBMPFromFile: file '%s', rotationX=%d, rotationY=%d, rotation=%d, erase=%d\n", fileName, rotationX, rotationY, rotation, erase);

    tft.startWrite();
    uint16_t bufferPosition = 0;
    for (int y = 0; y < bmpInfo.height; y++) {
      yield();
      for (int x = 0; x < bmpInfo.width; x++) {
        uint16_t color = getPixelColor(&bmpInfo, x, bmpInfo.height - y - 1);
        // only draw actual pixels. Color 0x0000 is handled as keeping the background
        if(color == 0) continue;
        
        if(erase) {
          // color 0x0000 erases the pixel and redraws the original color
          if(color != 0) {
            color = 0;
          } else {
            continue;
          }
        }
        switch (rotation) {
          case 0:
            tft.writePixel(xOffset - rotationX + x, yOffset - rotationY + y, color);
            break;
          case 90:
            tft.writePixel(xOffset - (bmpInfo.height - rotationY) + (bmpInfo.height - y), yOffset - rotationX + x, color);
            break;
          case 180:
            tft.writePixel(xOffset + rotationX - x, yOffset + rotationY - y, color);
            break;
          case 270:
            tft.writePixel(xOffset + (bmpInfo.height - rotationY) - (bmpInfo.height - y), yOffset + rotationX - x, color);
            break;
        }
      }
    }
    tft.endWrite();
    printFreeRam();
    free(bmpInfo.pixelBuffer);
  } else {
    Serial.print("drawBMPFromFile: Failed processing '");
    Serial.print(fileName);
    Serial.println("'");
  }
}

File bitmapFile;
long bitmapOffset;

void randomClockFace() {
  currentFaceNumber = random(1, NUMBER_OF_FACE_FILES + 1);

  readHandPositions("hoursHand.pos", hoursHandPositions);
  readHandPositions("minutesHand.pos", minutesHandPositions);
 
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "/%d/clockface.bmp", currentFaceNumber);
  if (initBMP(buffer)) {
    drawBMP(tft);
  }
}

boolean initBMP(char* fileName) {
  // Get the offset to the first bytes of the actual bitmap
  bitmapFile = LittleFS.open(fileName, "r");
  if (!bitmapFile) {
    Serial.printf("initBMP: Can't open '%s'\n", fileName);
    return false;
  }
  bitmapFile.seek(10);
  bitmapFile.read((uint8_t *)&bitmapOffset, sizeof(bitmapOffset));
  return true;
}

void drawBMP(ClockTFT &tft) {
  Serial.printf("drawBMP: bitmapOffset=%d\n", bitmapOffset);
  
  tft.startWrite();
  tft.setAddrWindow(0, 0, SCREEN_DIAMETER, SCREEN_DIAMETER);

  uint16_t buffer[SCREEN_DIAMETER];
  for (int y = 0; y < SCREEN_DIAMETER; y++) {
    yield();

    long seekPosition = bitmapOffset + (SCREEN_DIAMETER - y - 1) * SCREEN_DIAMETER * 2;
    bitmapFile.seek(seekPosition, SeekSet);

    bitmapFile.read((uint8_t *)buffer, SCREEN_DIAMETER * 2);
    tft.writePixels(buffer, SCREEN_DIAMETER);
  }
  tft.endWrite();
}

uint16_t readColor(int16_t x, int16_t y) {
  long seekPosition = bitmapOffset + ((SCREEN_DIAMETER - y - 1) * SCREEN_DIAMETER  + x) * 2;
  bitmapFile.seek(seekPosition, SeekSet);
  uint16_t color;
  bitmapFile.read((uint8_t *)&color, sizeof(color));
  return color;
}
