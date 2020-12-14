#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "env.h"

const char* ssid = AP_SSID;
const char* password = AP_PASSWORD;
const char* host = "iot.hongo.wide.ad.jp";
const int port = 10030;
const char* ntp_server = "ntp.nict.jp";
unsigned long last_sync_time = 0;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     2 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  Serial.begin(115200);

  pinMode(12, INPUT);  // For DIP Switch
  pinMode(13, INPUT);  // For DIP Switch
  pinMode(16, INPUT);  // For Motion Detector

  pinMode(14, OUTPUT); // For Buzzer

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  display.print("Connecting to ");
  display.println(ssid);
  display.display();

  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (true) {
    delay(500);
    wl_status_t status = WiFi.status();
    if (status == WL_IDLE_STATUS || status == WL_DISCONNECTED) {
      display.print(".");
      display.display();
      continue;
    } else if (status == WL_CONNECTED) {
      display.println("");
      display.println("success");
      display.display();
      success();
      break;
    } else {
      display.println("");
      display.println("failure");
      switch (status) {
        case WL_NO_SSID_AVAIL:
          display.println("no ssid available");
          break;
        case WL_CONNECT_FAILED:
          display.println("wrong password");
          break;
      }
      display.display();
      failure();
      return;
    }
  }

  display.print("IP address: ");
  display.println(WiFi.localIP());
  display.display();

  display.print("Sync to ");
  display.println(ntp_server);
  if (syncNTPtime(ntp_server)) {
    display.println("success");
    display.display();
    success();
  } else {
    display.println("failure");
    display.display();
    failure();
    return;
  }

  unsigned long prev_t = 0;
  while (true) {
    unsigned long t = now();

    if (t / 30 != prev_t / 30) {
      int id = getDIPSWStatus();
      char str_time[30];
      sprintf(str_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                      year(t), month(t), day(t),
                      hour(t), minute(t), second(t));
      int illuminance = getIlluminance();
      int md_status = getMDStatus();

      char send_buf[100];
      sprintf(send_buf, "%d,%s,%d,%d\r\n",
        id, str_time, illuminance, md_status
      );

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(send_buf);
      display.display();

      // Use WiFiClient class to create TCP connections
      WiFiClient client;

      if (!client.connect(host, port)) {
        display.println("...ERR");
        display.display();
        failure();
        return;
      }

      // This will send the request to the server
      client.print(send_buf);
      delay(1000);
      if (!client.connected()) {
        display.println("...ERR");
        display.display();
        failure();
        return;
      }

      String line = client.readString();

      if (line == String("OK\r\n")) {
        display.println("...OK");
        display.display();
        success();
      } else {
        display.println("...NG");
        display.display();
        failure();
      }

      client.stop();
    }

    prev_t = t;

    if (t / 300 != last_sync_time / 300) {
      syncNTPtime(ntp_server);
      last_sync_time = t;
    }
    delay(500);

  }
}

void loop() {
}

void success() {
  setBZ(true);
  delay(20);
  setBZ(false);
}

void failure() {
  setBZ(true);
  delay(1000);
  setBZ(false);
}
