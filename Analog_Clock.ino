#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <TZ.h>
#include <coredecls.h> // required for settimeofday_cb() (NTP sync callback)

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "ClockTFT.h"
#include "defines.h"
#include "clock.h"

boolean timeIsSet = false;
time_t lastNtpSet = 0;
time_t currentTime = time(nullptr); // time_t = seconds since epoch
struct tm * timeinfo;
char ssid[33];
char wifiPassword[64];

WiFiEventHandler gotIpEventHandler;
WiFiEventHandler disconnectedEventHandler;
unsigned long lastDisconnectTime = 0;
bool isDisconnected = false;

ClockTFT tft(TFT_CS, TFT_DC, TFT_RST);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

uint8_t currentFaceNumber;
HandPosition hoursHandPositions[15];
HandPosition minutesHandPositions[15];

// This overrides the default 1-hour interval (in milliseconds)
// Note: NTP servers usually request a minimum of 15 seconds between polls.
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000() {
    return 10 * 60 * 1000UL; // Example: Update every 10 minutes instead
}

// Record the NPT set time
void timeUpdated() {
  timeIsSet = true;
  lastNtpSet = time(nullptr);
  Serial.printf("NTP Updated: %s\n", ctime(&lastNtpSet));
}

void printFreeRam() {
  Serial.printf("Free ram: %d bytes\n", ESP.getFreeHeap());
}

void listAllFilesInDir(const char *dir_path)
{
  Dir dir = LittleFS.openDir(dir_path);
  while(dir.next()) {
    if (dir.isFile()) {
      Serial.printf("   %s - %d\n", dir.fileName().c_str(), (int)dir.fileSize());
    }
    if (dir.isDirectory()) {
      char subDir[64];
      snprintf(subDir, sizeof(subDir), "%s%s/", dir_path, dir.fileName().c_str());
      Serial.printf("Dir: %s\n", subDir);
      listAllFilesInDir(subDir);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial attach
  Serial.println();
  delay(5000);
  Serial.printf("\n\n");

  Serial.println(F("TFT pin info:"));
  Serial.printf_P(PSTR("  TFT_DIN: %d\n"), TFT_DIN);
  Serial.printf_P(PSTR("  TFT_CLK: %d\n"), TFT_CLK);
  Serial.printf_P(PSTR("  TFT_CS : %d\n"), TFT_CS);
  Serial.printf_P(PSTR("  TFT_DC : %d\n"), TFT_DC);
  Serial.printf_P(PSTR("  TFT_RST: %d\n"), TFT_RST);
  Serial.printf_P(PSTR("  TFT_BL : %d\n"), TFT_BL);

  Serial.printf("Flash size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("CPU       : %d MHz\n", ESP.getCpuFreqMHz());
  printFreeRam();

  // For the ESP the flash has to be read to a buffer
  EEPROM.begin(128);

  // Setup the LCD
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);

  // Setup the wifi
  EEPROM.get(SSID_ADDR, ssid);
  EEPROM.get(WIFI_PASSWORD_ADDR, wifiPassword);

  // Clean string arrays ensuring a null terminator exists
  ssid[32] = '\0';
  wifiPassword[63] = '\0';

  Serial.printf("\nConnecting to WIFI '%s'... ", ssid);
  tft.fillCircle(clock_center_x, clock_center_y, SCREEN_DIAMETER / 10, GC9A01A_BLUE);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  // Register Event Handlers
  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
    Serial.print("WiFi Restored! IP address: ");
    Serial.println(event.ip); // Use event data directly
    isDisconnected = false;
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
    Serial.printf("Disconnected! Reason: %u\n", event.reason);
    
    // Set baseline tracking when the disconnect first occurs
    if (!isDisconnected) {
      isDisconnected = true;
      lastDisconnectTime = millis();
    }
  });

  WiFi.begin(ssid, wifiPassword);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Continuing...");
    tft.fillCircle(clock_center_x, clock_center_y, SCREEN_DIAMETER / 10, GC9A01A_RED);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(HOSTNAME);
  } else {
    Serial.println("Connected to WIFI...");
    tft.fillCircle(clock_center_x, clock_center_y, SCREEN_DIAMETER / 10, GC9A01A_GREEN);
  }
  Serial.print("IP address: ");
  String ipAddress = WiFi.localIP().toString();
  Serial.println(ipAddress);

  tft.setTextSize(2);
  int16_t xPos, yPos;
  uint16_t width, height;
  tft.getTextBounds(ipAddress, 0, 100, &xPos, &yPos, &width, &height);
  tft.setTextColor(GC9A01A_WHITE);
  tft.setCursor(clock_center_x - width / 2, clock_center_y + 50);
  tft.println(ipAddress);
  delay(1000);

  // implement NTP update of timekeeping (with automatic hourly updates)
  configTime(MY_TZ, NTP_SERVERS);

  // callback, when NTP changes the time
  settimeofday_cb(timeUpdated);

  setupOTA();

  // Setup the web server
  httpUpdater.setup(&server);
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.begin();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS problem");
    tft.fillScreen(GC9A01A_BLACK);
    String spiffsError = "SPIFFS error";
    tft.getTextBounds(spiffsError, 0, 100, &xPos, &yPos, &width, &height);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setCursor(clock_center_x - width / 2, clock_center_y);
    tft.println(spiffsError);
    delay(1000);
  }

  // Show some FS info
  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.println("LittleFS info:");
  Serial.printf("  Total space     : %d bytes\n", fs_info.totalBytes);
  Serial.printf("  Total space used: %d bytes\n", fs_info.usedBytes);
  Serial.printf("  Block size      : %d bytes\n", fs_info.blockSize);
  Serial.printf("  Page size       : %d bytes\n", fs_info.pageSize);
  Serial.printf("  Max open files  : %d\n", fs_info.maxOpenFiles);
  Serial.printf("  Max path length : %d bytes\n", fs_info.maxPathLength);
  //listAllFilesInDir("/");

  // Initialize face
  randomClockFace();

  Serial.println();
  Serial.println("Running...");
}

void loop() {
  // Non-blocking fallback mechanism if the built-in auto-reconnect gets stuck
  if (isDisconnected && (millis() - lastDisconnectTime >= RECONNECT_INTERVAL)) {
    Serial.println("Built-in reconnect stuck. Forcing manual reconnection retry...");
    lastDisconnectTime = millis();
    
    WiFi.disconnect(); // Clear dead state
    WiFi.begin(ssid, wifiPassword); // Force fresh attempt
  }

  ArduinoOTA.handle();
  server.handleClient();

  updateClock();
  
  static time_t last_check_time = -1;
  time_t now = time(nullptr);
  if (now != last_check_time && now - (3600 * 24) > last_check_time) {
    // run once a day
    last_check_time = now;
    //check_for_updates();
  }
}
