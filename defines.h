#define PASSWORD "RoundClockPassword"
#define HOSTNAME "RondKlokje"

#define SSID_ADDR 0 // String[60]
#define WIFI_PASSWORD_ADDR (SSID_ADDR + sizeof(ssid)) // String[60]
#define BRIGHTNESS_ADDR (WIFI_PASSWORD_ADDR + sizeof(wifiPassword)) // int

// The timezone to sync the date/time to, using NTP. For timezone to use, see TZ.h.
#define MY_TZ TZ_Europe_Amsterdam

// NTP server list to use for syncing time.
#define NTP_SERVERS "0.nl.pool.ntp.org", "1.nl.pool.ntp.org", "2.nl.pool.ntp.org"

#define NUMBER_OF_FACE_FILES 3

#define TFT_VCC 3V3
#define TFT_GND GND
#define TFT_DIN D7
#define TFT_CLK D5
#define TFT_CS  D0
#define TFT_DC  D3
#define TFT_RST D4
#define TFT_BL  D1

#define SCREEN_DIAMETER 240

#define SECOND_SPLIT 5

const int clock_center_x = SCREEN_DIAMETER / 2;
const int clock_center_y = SCREEN_DIAMETER / 2;

const float pi = 3.14159267 ;
  
#define CSS "<style>\
  body {\
    background: #fad7a0;\
    color: #154360;\
    padding: 20px;\
    font-size: 3em;\
    text-align: center;\
  }\
  div.container {\
    display: inline-block;\
    width: 90%;\
    height: 90%;\
    background: #f8c471;\
    box-shadow: 15px 20px 20px #88888888;\
    border-radius: 20px;\
    padding: 2%;\
    text-align: left;\
  }\
  h1 {\
    margin-top: 0;\
  }\
  input {\
    width: 100%;\
    border: 0;\
    border-bottom: 2px solid grey;\
    background: none;\
    color: #154360;\
    font-size: 1.2em;\
  }\
  input[type=\"range\"] {\
    width: 100%;\
  }\
  input[type=\"submit\"] {\
    background: #154360;\
    color: #fad7a0;\
    border: 0;\
    border-radius: 5px;\
    width: 40%;\
    height: 10%;\
    cursor: pointer;\
    font-size: 1em;\
    position: absolute;\
    left: 30%;\
    bottom: 20%;\
  }\
  div div {\
    position: absolute;\
    right: 2%;\
    bottom: 2%;\
    font-size: .6em;\
  }\
</style>"
