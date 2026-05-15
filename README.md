# Analog Clock for 1.28" round display

## Update CA
- echo | openssl s_client -servername raw.githubusercontent.com -connect raw.githubusercontent.com:443 -showcerts | grep -E "subject=|issuer="
- Take highest depth: depth=2 C=US, O=Internet Security Research Group, CN=ISRG Root X1
  => ISRG Root X1
  => cat /etc/ssl/certs/ISRG_Root_X1.pem
- use this cert as the CA

## Setup
- Write the LittleFS
- Write the program using the Arduino IDE
- Connect your computer or phone to th 'RondKlokje' wifi network
- Open http://192.168.4.1
- Goto 'wifi' and enter the credentials

# Info
- [GC9A01A Arduino Library](https://github.com/PaintYourDragon/Adafruit_GC9A01A)

# BMP files
All files must have a width that is a multiple of 4. This is because the BMP encoding pads all rows 
to a 32 bit size.

# LittleFS
- [Arduino ESP8266 LittleFS Filesystem Upload](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin)
- [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)

## Efficient LittleFS
It would be nice to be able to change the blocksize. But doing the thing described below doesn't seem to work.
Probably the implementation of LittleFS does not support this. I don't know.

To allow for more efficient use of the storage change the blocksize in:

`boards.txt` (`~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/boards.txt`):

   `d1_mini.menu.eesz.4M3M.build.spiffs_blocksize=4096`

Maybe also this file `eagle.flash.4m.ld` (`~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/sdk/ld/eagle.flash.4m.ld`):

   `PROVIDE ( _SPIFFS_start = 0x40308000 ); 
   PROVIDE ( _SPIFFS_block = 0x1000 );`

