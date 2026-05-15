#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>

#define UPDATE_HOST "raw.githubusercontent.com"
#define UPDATE_PORT 443
#define VERSION_FILE_LOCATION "/MilovdZee/Analog_Clock/master/firmware/version.txt"
#define FIRMWARE_URL "https://" UPDATE_HOST "/MilovdZee/Analog_Clock/master/firmware/Analog_Clock.ino-%d.bin"

#define CONNECTION_TIMEOUT 10000

BearSSL::WiFiClientSecure client;

// Root CA stored cleanly in Flash memory
const char *root_ca PROGMEM =
  "-----BEGIN CERTIFICATE-----\n"
  "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\n"
  "-----END CERTIFICATE-----\n";

BearSSL::X509List trustedRootCert(root_ca);

static void setup_client() {
  client.setBufferSizes(8192, 512);
  client.setTrustAnchors(&trustedRootCert);
}

boolean check_for_data() {
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > CONNECTION_TIMEOUT) {
      Serial.println(F("Client Timeout!"));
      return false;
    }
    delay(10);
  }
  return true;
}

long read_content_length_from_headers() {
  long content_length = ERROR_VALUE;
  char line_buf[128];

  while (client.available()) {
    int bytes_read = client.readBytesUntil('\n', line_buf, sizeof(line_buf) - 1);
    line_buf[bytes_read] = '\0';

    String line(line_buf);
    line.trim();

    if (line.length() == 0) break;

    if (line.startsWith(F("HTTP/1.1")) || line.startsWith(F("HTTP/1.0"))) {
      if (line.indexOf(F("200")) < 0) {
        Serial.printf_P(PSTR("Got a non 200 status code: '%s'\n"), line.c_str());
        return ERROR_VALUE;
      }
    }

    if (line.startsWith(F("Content-Length: "))) {
      content_length = atol(line.substring(16).c_str());
      Serial.printf_P(PSTR("  - Content length: %ld bytes\n"), content_length);
    }
  }
  return content_length;
}

int get_update_version() {
  setup_client();

  if (!client.connect(UPDATE_HOST, UPDATE_PORT)) {
    Serial.printf_P(PSTR("Connection failed: '%s:%d'\n"), UPDATE_HOST, UPDATE_PORT);
    return ERROR_VALUE;
  }
  Serial.printf_P(PSTR("Requesting '%s'\n"), VERSION_FILE_LOCATION);

  client.print(F("GET "));
  client.print(VERSION_FILE_LOCATION);
  client.print(F(" HTTP/1.0\r\nHost: "));
  client.print(UPDATE_HOST);
  client.print(F("\r\nUser-Agent: ESP8266\r\nCache-Control: no-cache\r\n\r\n"));

  int newest_version = ERROR_VALUE;
  if (check_for_data()) {
    long content_length = read_content_length_from_headers();
    if (content_length != ERROR_VALUE && client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      newest_version = atoi(line.c_str());
    }
  }
  Serial.printf_P(PSTR("Newest firmware version: %d\n"), newest_version);

  client.stop();

  if (newest_version < 1 || newest_version > 100) return ERROR_VALUE;
  return newest_version;
}

void update_firmware(int version) {
  char url[128];
  snprintf_P(url, sizeof(url), PSTR(FIRMWARE_URL), version);
  Serial.printf_P(PSTR("Downloading: %s\n"), url);

  setup_client();

  ESPhttpUpdate.onProgress([](int progress, int total) {
    ota_on_progress(progress, total);
  });

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, String(url));

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf_P(PSTR("Update failed (%d): %s\n"),
                      ESPhttpUpdate.getLastError(),
                      ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("No update available"));
      break;
    case HTTP_UPDATE_OK:
      Serial.println(F("Update OK"));
      break;
  }
}

void check_for_updates() {
  if (WiFi.status() == WL_CONNECTED) {
    delay(500);
    Serial.printf_P(PSTR("Checking for firmware updates... (installed v%d)\n"), CURRENT_FIRMWARE_VERSION);
    int newest_version = get_update_version();
    if (newest_version > CURRENT_FIRMWARE_VERSION) {
      Serial.println(F("Updating firmware..."));
      update_firmware(newest_version);
    }
  }
}
