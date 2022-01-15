# Analog Clock for 1.28" round display

# Info
- [GC9A01A Arduino Library](https://github.com/PaintYourDragon/Adafruit_GC9A01A)

# BMP files
All files must have a width that is a multiple of 4. This is because the BMP encoding pads all rows 
to a 32 bit size.

# LittleFS
- [Arduino ESP8266 LittleFS Filesystem Uploade](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin)

## Efficient LittleFS
To allow for more efficient use of the storage change the blocksize in:

boards.txt:
   d1_mini.menu.eesz.4M3M.build.spiffs_blocksize=4096

Maybe also this fil `eagle.flash.4m.ld`:
   PROVIDE ( _SPIFFS_start = 0x40308000 ); PROVIDE ( _SPIFFS_block = 0x1000 );

(This does not seem to work) 
