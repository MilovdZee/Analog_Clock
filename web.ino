void handleRoot() {
  String rootForm = String(CSS) + "<html>\
  <body>\
    <div class=\"container\">\
      <h1>Settings</h1>\
      <div><a href=\"/wifi\">wifi</a></div>\
    </div>\
  </body>\
</html>";

  server.send(200, "text/html", rootForm);
}

void handleWifi() {
  if (server.method() == HTTP_POST) {
    tft.fillScreen(GC9A01A_BLACK);
    for (uint16_t degrees = 0; degrees < 360; degrees += 10) {
      float angle = 2.0 * pi / 360 * degrees;
      int x = clock_center_x + (SCREEN_DIAMETER / 2 - 50) * sin(angle);
      int y = clock_center_y - (SCREEN_DIAMETER / 2 - 50) * cos(angle);
      tft.fillCircle(x, y, 10, GC9A01A_YELLOW);
      delay(50);
    }
    delay(500);

    snprintf(ssid, sizeof(ssid), server.arg("ssid").c_str());
    snprintf(wifiPassword, sizeof(wifiPassword), server.arg("password").c_str());

    // Store values in EEProm
    EEPROM.put(SSID_ADDR, ssid);
    if (String(wifiPassword).length() > 0) EEPROM.put(WIFI_PASSWORD_ADDR, wifiPassword);
    EEPROM.commit();
    
    delay(500);
    ESP.restart();
  }

  // Read back to check if the values are stored correctly
  EEPROM.get(SSID_ADDR, ssid);
  EEPROM.get(WIFI_PASSWORD_ADDR, wifiPassword);

  String wifiForm = String(CSS) + "<html>\
  <body>\
    <div class=\"container\">\
    <h1>WiFi Settings</h1>\
      <form method=\"POST\" action=\"/wifi\">\
        SSID:</br>\
        <input type=\"text\" name=\"ssid\" value=\"" + String(ssid) + "\"></br></br>\
        Password:</br>\
        <input type=\"text\" name=\"password\" value=\"" + String(wifiPassword) + "\">\
        <input type=\"submit\" value=\"Submit\">\
      </form>\
    </div>\
  </body>\
</html>";

  server.send(200, "text/html", wifiForm);
}
