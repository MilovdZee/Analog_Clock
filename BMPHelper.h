class BMPInfo {
  public:
    uint16_t width;
    uint16_t paddedWidth;
    uint16_t height;
    uint16_t bitsPerPixel;
    uint16_t colorTable[2];
    uint8_t *pixelBuffer;
};

boolean readBMPFile(char *fileName, BMPInfo *bmpFile);
uint16_t getPixelColor(BMPInfo *bmpInfo, uint8_t x, uint8_t y);
