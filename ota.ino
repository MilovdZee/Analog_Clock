void setupOTA() {
  // Setup OTA updates
  ArduinoOTA.setPort(8266); // default 8266
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "%s_%d", HOSTNAME, random(9999));
  ArduinoOTA.setHostname(buffer);
  ArduinoOTA.setPassword(PASSWORD); // No authentication by default

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update start");
    tft.fillScreen(GC9A01A_BLACK);
    tft.fillCircle(clock_center_x, clock_center_y, 50, GC9A01A_BLUE);
    delay(1000);
    tft.fillScreen(GC9A01A_BLACK);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update end");
    tft.fillScreen(GC9A01A_BLACK);
    tft.fillCircle(clock_center_x, clock_center_y, 50, GC9A01A_GREEN);
    delay(1000);
    tft.fillScreen(GC9A01A_BLACK);
  });
  ArduinoOTA.onProgress([](int progress, int total) {
    int percentage = progress * 100 / total;
    Serial.printf("OTA update progress: %u\r\n", percentage);
    
    float angle = 2.0 * pi / 110.0 * percentage;
    int x = clock_center_x + (SCREEN_DIAMETER / 2 - 50) * sin(angle);
    int y = clock_center_y - (SCREEN_DIAMETER / 2 - 50) * cos(angle);
    tft.fillCircle(x, y, 10, GC9A01A_YELLOW);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");

    tft.fillCircle(clock_center_x, clock_center_y, 50, GC9A01A_RED);

    tft.setTextSize(3);
    int16_t xPos, yPos;
    uint16_t width, height;
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%s", error);
    tft.getTextBounds(buffer, 0, 100, &xPos, &yPos, &width, &height);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setCursor(clock_center_x - width / 2, clock_center_y - height / 2);
    tft.println(buffer);

    delay(1000);
  });
  ArduinoOTA.begin();
}
