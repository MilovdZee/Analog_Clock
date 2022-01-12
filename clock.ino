void draw_second(int splittedSecond, int mode) {
  int x = ((SCREEN_DIAMETER - 55) / 2 * sin(pi - (2 * pi) / 60 / SECOND_SPLIT * splittedSecond)) + clock_center_x;
  int y = ((SCREEN_DIAMETER - 55) / 2 * cos(pi - (2 * pi) / 60 / SECOND_SPLIT * splittedSecond)) + clock_center_y;
  if (mode == 1)
    tft.fillCircle(x, y, 4, GC9A01A_DARKGREEN);
  else
    tft.fillCircle(x, y, 4, GC9A01A_BLACK);
}

void draw_minute(int minute, int mode) {
  int x = ((SCREEN_DIAMETER - 100) / 2 * sin(pi - (2 * pi) / 60 * minute)) + clock_center_x;
  int y = ((SCREEN_DIAMETER - 100) / 2 * cos(pi - (2 * pi) / 60 * minute)) + clock_center_y;
  if (mode == 1)
    tft.writeFatLine(clock_center_x, clock_center_y, x, y, GC9A01A_BLUE, 2);
  else
    tft.writeFatLine(clock_center_x, clock_center_y, x, y, GC9A01A_BLACK, 2);
}

void draw_hour(int hour, int minute, int mode) {
  int y = ((SCREEN_DIAMETER - 140) / 2 * cos(pi - (2 * pi) / 12 * hour - (2 * PI) / 720 * minute)) + clock_center_y;
  int x = ((SCREEN_DIAMETER - 140) / 2 * sin(pi - (2 * pi) / 12 * hour - (2 * PI) / 720 * minute)) + clock_center_x;
  if (mode == 1)
    tft.writeFatLine(clock_center_x, clock_center_y, x, y, GC9A01A_RED, 2);
  else
    tft.writeFatLine(clock_center_x, clock_center_y, x, y, GC9A01A_BLACK, 2);
}

void draw_clock_face(void) {
  tft.fillScreen(GC9A01A_BLACK);

  // draw hour pointers around the face of a clock
  for (int i = 1; i < 12; i++) {
    int x1 = (SCREEN_DIAMETER / 2 * sin(pi - (2 * pi) / 12 * i)) + clock_center_x;
    int y1 = (SCREEN_DIAMETER / 2 * cos(pi - (2 * pi) / 12 * i)) + clock_center_y;
    int x2 = ((SCREEN_DIAMETER - 12) / 2 * sin(pi - (2 * pi) / 12 * i)) + clock_center_x;
    int y2 = ((SCREEN_DIAMETER - 12) / 2 * cos(pi - (2 * pi) / 12 * i)) + clock_center_y;
    tft.drawLine(x1, y1, x2, y2, GC9A01A_WHITE);
  }

  // print string "12" at the top of the face of the clock
  tft.setTextSize(1);
  tft.setTextColor(GC9A01A_WHITE);
  tft.setCursor(clock_center_x - 5, 0);
  tft.println(F("12"));
}

void redraw_clock_face_elements(void) {
  tft.drawCircle(clock_center_x, clock_center_y, 3, GC9A01A_WHITE);
  tft.fillCircle(clock_center_x, clock_center_y, 3, GC9A01A_WHITE);
  tft.setCursor(clock_center_x - 3, 0);
  tft.println(F("12"));
}

// Check if NTPD sync was more than ONE hour and 20 SECONDS ago. If so, we show it on screen to indicate we are not sure about the time yet or anymore.
boolean isNtpOlderThanOneHour() {
  // time(nullptr) = time in seconds
  return (!timeIsSet) || (time(nullptr) - lastNtpSet) > 3620;
}

int hour = -1;
int minute = -1;
int splittedSecond = -1;

boolean ntpMissed = false;

int previousClockSecond = -1;
int millisOffset = 0; // Offset compared to millis() to get partial seconds in sync with the NTP seconds

void updateClock() {
  currentTime = time(nullptr); // time_t = seconds since epoch

  if (isNtpOlderThanOneHour()) {
    ntpMissed = true;
    // No fresh NTP time info? Hide clock hands...
    if (currentTime % 2 == 0) {
      // Flashing red dot in the middle to indicate loss of time
      tft.fillCircle(clock_center_x, clock_center_y, 10, GC9A01A_RED);
    } else {
      tft.fillCircle(clock_center_x, clock_center_y, 10, GC9A01A_BLACK);
    }
    return;
  }

  if(ntpMissed == true) {
    ntpMissed = false;
    tft.fillCircle(clock_center_x, clock_center_y, 10, GC9A01A_BLACK);
  }

  timeinfo = localtime (&currentTime); // setup timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec
  int millisOfSecond = millis() % 1000L;
  if (previousClockSecond != timeinfo->tm_sec) {
    // Reset the millis offset
    millisOffset = -millisOfSecond;
    previousClockSecond = timeinfo->tm_sec;
  }
  millisOfSecond = millisOfSecond + millisOffset;
  if (millisOfSecond < 0) millisOfSecond += 1000;

  int newSplittedSecond = timeinfo->tm_sec * SECOND_SPLIT + millisOfSecond * SECOND_SPLIT / 1000;
  if (newSplittedSecond > splittedSecond || splittedSecond - newSplittedSecond > 30) {
    // Second check to prevent some weird jumps
    draw_second(splittedSecond, 0);
    splittedSecond = newSplittedSecond;
    draw_second(splittedSecond, 1);
  }

  if (timeinfo->tm_min != minute) {
    if(minute % 5 == 0) randomBMP("/clockface%d.bmp");
    
    tft.startWrite();
    
    draw_minute(minute, 0);
    draw_hour(hour, minute, 0);
    
    hour = timeinfo->tm_hour;
    minute = timeinfo->tm_min;
    
    draw_minute(minute, 1);
    draw_hour(hour, minute, 1);
    
    tft.endWrite();
    
    tft.fillCircle(clock_center_x, clock_center_y, 5, GC9A01A_RED);
  }
}
