static const char CSS_P[] PROGMEM =
  "<style>"
  "body{background:#fad7a0;color:#154360;padding:20px;font-size:3em;text-align:center;}"
  "div.container{display:inline-block;width:90%;height:90%;background:#f8c471;"
  "box-shadow:15px 20px 20px #88888888;border-radius:20px;padding:2%;text-align:left;}"
  "h1{margin-top:0;}"
  "input{width:100%;border:0;border-bottom:2px solid grey;background:none;"
  "color:#154360;font-size:1.2em;}"
  "input[type=\"range\"]{width:100%;}"
  "input[type=\"submit\"]{background:#154360;color:#fad7a0;border:0;border-radius:5px;"
  "width:40%;height:10%;cursor:pointer;font-size:1em;position:absolute;left:30%;bottom:20%;}"
  "div div{position:absolute;right:2%;bottom:2%;font-size:.6em;}"
  "</style>";

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent_P(CSS_P);
  server.sendContent_P(PSTR(
    "<html><body><div class=\"container\">"
    "<h1>Settings</h1>"
    "<div><a href=\"/wifi\">wifi</a></div>"
    "</div></body></html>"));
}

void handleWifi() {
  if (server.method() == HTTP_POST) {
    tft.fillScreen(GC9A01A_BLACK);
    for (uint16_t degrees = 0; degrees < 360; degrees += 10) {
      float angle = 2.0f * pi / 360 * degrees;
      int x = clock_center_x + (SCREEN_DIAMETER / 2 - 50) * sin(angle);
      int y = clock_center_y - (SCREEN_DIAMETER / 2 - 50) * cos(angle);
      tft.fillCircle(x, y, 10, GC9A01A_YELLOW);
      delay(50);
    }
    delay(500);

    snprintf(ssid, sizeof(ssid), "%s", server.arg("ssid").c_str());
    snprintf(wifiPassword, sizeof(wifiPassword), "%s", server.arg("password").c_str());

    EEPROM.put(SSID_ADDR, ssid);
    if (wifiPassword[0] != '\0') EEPROM.put(WIFI_PASSWORD_ADDR, wifiPassword);
    EEPROM.commit();

    delay(500);
    ESP.restart();
  }

  EEPROM.get(SSID_ADDR, ssid);
  EEPROM.get(WIFI_PASSWORD_ADDR, wifiPassword);

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent_P(CSS_P);
  server.sendContent_P(PSTR(
    "<html><body><div class=\"container\">"
    "<h1>WiFi Settings</h1>"
    "<form method=\"POST\" action=\"/wifi\">"
    "SSID:</br>"
    "<input type=\"text\" name=\"ssid\" value=\""));
  server.sendContent(ssid);
  server.sendContent_P(PSTR("\"></br></br>Password:</br>"
    "<input type=\"text\" name=\"password\" value=\""));
  server.sendContent(wifiPassword);
  server.sendContent_P(PSTR("\">"
    "<input type=\"submit\" value=\"Submit\">"
    "</form></div></body></html>"));
}
