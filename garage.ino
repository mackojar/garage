#include <stdio.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "credentials.h"
#include "rest.h"

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

#define STATUS_LED_TIMEOUT 5000
#define STATUS_LED_BLINK_TIME 150

unsigned long lastStatusBlinkTimeOn;
unsigned long lastStatusBlinkTimeOff;

ESP8266WebServer server(HTTP_REST_PORT);

int initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
}

void initStatusLED() {
  lastStatusBlinkTimeOn = millis() + STATUS_LED_TIMEOUT;
  lastStatusBlinkTimeOff = lastStatusBlinkTimeOn + STATUS_LED_BLINK_TIME;
}

void initRestServer() {
  server.begin();
  Serial.println("HTTP REST Server Started");
}

int getWIFIBlinkState( int num) {
  return num & 0x01;  // LOW and HIGH...
}

void connectWIFI() {
  WiFi.disconnect();
  initWifi();
  Serial.println();
  Serial.print("Connecting to WiFi AP...");
  unsigned int retries = 0;  // force first getBlinkState to return LOW -> inverted (LOW->LED is on)
  digitalWrite( LED_BUILTIN, getWIFIBlinkState( retries));
  while ((WiFi.status() != WL_CONNECTED) /*&& (retries < MAX_WIFI_INIT_RETRY) */) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    digitalWrite( LED_BUILTIN, getWIFIBlinkState( retries));
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connetted to ");
  Serial.print(WIFI_SSID);
  Serial.print(", IP: ");
  Serial.println(WiFi.localIP());

  digitalWrite( LED_BUILTIN, HIGH);
}

void checkWIFI() {
  if( WiFi.status() != WL_CONNECTED) {
    server.close();
    connectWIFI();
    initStatusLED();
    initRestServer();
  } else {
    blinkStatusLED();
  }
}

void blinkStatusLED() {
  unsigned long currentMillis = millis();
  if( (long)(currentMillis - lastStatusBlinkTimeOn) >= STATUS_LED_TIMEOUT) {
    digitalWrite( LED_BUILTIN, LOW);
    lastStatusBlinkTimeOn += STATUS_LED_TIMEOUT;
  } else if( (long)(currentMillis - lastStatusBlinkTimeOff) >= STATUS_LED_TIMEOUT) {
    digitalWrite( LED_BUILTIN, HIGH);
    lastStatusBlinkTimeOff += STATUS_LED_TIMEOUT;
  }
}

void setup(void) {
  Serial.begin(115200);
  bool mounted = SPIFFS.begin();
  if( mounted) {
    Serial.println("SPIFFS mounted.");
  } else {
    Serial.println("Error mounting SPIFFS");
  }

  pinMode( PIN_GARAGE_DOOR, OUTPUT);
  pinMode( LED_BUILTIN, OUTPUT);    

  config_rest_server_routing();
}

void loop(void) {
  checkWIFI();
  server.handleClient();
}
