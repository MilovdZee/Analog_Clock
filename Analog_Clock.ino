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

boolean timeIsSet = false;
time_t lastNtpSet = 0;
time_t currentTime = time(nullptr); // time_t = seconds since epoch
struct tm * timeinfo;
time_t previousEffectTime = time(nullptr);

char ssid[60];
char wifiPassword[60];

int numberOfFacesFiles = 0;

ClockTFT tft(TFT_CS, TFT_DC, TFT_RST);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Record the NPT set time
void timeUpdated() {
  timeIsSet = true;
  lastNtpSet = time(nullptr);
  Serial.print("NTP Updated: "); Serial.println(ctime(&lastNtpSet));
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  // For the ESP the flash has to be read to a buffer
  EEPROM.begin(512);

  // Setup the LCD
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);

  // Setup the wifi
  EEPROM.get(SSID_ADDR, ssid);
  EEPROM.get(WIFI_PASSWORD_ADDR, wifiPassword);
  Serial.print("\r\nConnecting to WIFI '");
  Serial.print(String(ssid));
  Serial.print("'...\r\n");
  tft.fillCircle(clock_center_x, clock_center_y, SCREEN_DIAMETER / 10, GC9A01A_BLUE);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(String(ssid), String(wifiPassword));
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
    tft.fillScreen(GC9A01A_BLACK);
    String spiffsError = "SPIFFS error";
    tft.getTextBounds(spiffsError, 0, 100, &xPos, &yPos, &width, &height);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setCursor(clock_center_x - width / 2, clock_center_y);
    tft.println(spiffsError);
    delay(1000);
  }

  // Count the number of faces
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) numberOfFacesFiles++;

  // Initialize face
  randomBMP("/clockface%d.bmp");

  Serial.println();
  Serial.println("Running...");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  updateClock();
}
