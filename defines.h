#define CURRENT_FIRMWARE_VERSION 1

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

#define RECONNECT_INTERVAL 30000UL

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

#define BUFFER_SIZE 128 // Small buffer configuration minimizes stack footprints
#define ERROR_VALUE -1

constexpr int clock_center_x = SCREEN_DIAMETER / 2;
constexpr int clock_center_y = SCREEN_DIAMETER / 2;

constexpr float pi = 3.14159267f;
  
