// File MUST be SCREEN_DIAMETER x SCREEN_DIAMETER pixels in 565 format uncompressed

File bitmapFile;
long bitmapOffset;

void randomBMP(char *filename) {
  int number = random(1, numberOfFacesFiles + 1);
  char buffer[50];
  snprintf(buffer, sizeof(buffer), filename, number);
  initBMP(buffer);
  drawBMP(tft);
}

void initBMP(char* filename) {
  // Get the offset to the first bytes of the actual bitmap
  bitmapFile = LittleFS.open(filename, "r");
  bitmapFile.seek(10);
  bitmapFile.read((uint8_t *)&bitmapOffset, sizeof(bitmapOffset));
}

void drawBMP(Adafruit_SPITFT &tft) {
  tft.startWrite();
  tft.setAddrWindow(0, 0, SCREEN_DIAMETER, SCREEN_DIAMETER);
  
  uint16_t buffer[SCREEN_DIAMETER];
  for(int y = 0; y < SCREEN_DIAMETER; y++) {
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
