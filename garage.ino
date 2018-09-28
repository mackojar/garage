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

IPAddress staticIP(192,168,1,22);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

void initStatusLED() {
  lastStatusBlinkTimeOn = millis() + STATUS_LED_TIMEOUT;
  lastStatusBlinkTimeOff = lastStatusBlinkTimeOn + STATUS_LED_BLINK_TIME;
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

int getBlinkState( int num) {
  if( num % 2 == 0) {
    return LOW;
  } else {
    return HIGH;
  }
}

int init_wifi() {
  int retries = 0;

  Serial.print("Connecting to WiFi AP...");
  digitalWrite( LED_BUILTIN, getBlinkState( retries));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  WiFi.config(staticIP, gateway, subnet);
  
  while ((WiFi.status() != WL_CONNECTED) /*&& (retries < MAX_WIFI_INIT_RETRY) */) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    digitalWrite( LED_BUILTIN, getBlinkState( retries));
    Serial.print(".");
  }
  digitalWrite( LED_BUILTIN, HIGH);
  return WiFi.status(); // return the WiFi connection status
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
    
    if (init_wifi() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Connetted to ");
        Serial.print(WIFI_SSID);
        Serial.print(", IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.print("Error connecting to: ");
        Serial.println(WIFI_SSID);
    }

    config_rest_server_routing();

    server.begin();
    Serial.println("HTTP REST Server Started");

    initStatusLED();
}

void loop(void) {
    server.handleClient();
    blinkStatusLED();
}

