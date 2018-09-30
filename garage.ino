#include <stdio.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <time.h>
#include <Ticker.h>
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

Ticker rainTrigger;

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

int initWIFIImpl() {
  int retries = 0;

  Serial.print("Connecting to WiFi AP...");
  digitalWrite( LED_BUILTIN, getBlinkState( retries));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  WiFi.config(staticIP, gateway, subnet, gateway);
  
  while ((WiFi.status() != WL_CONNECTED) /*&& (retries < MAX_WIFI_INIT_RETRY) */) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    digitalWrite( LED_BUILTIN, getBlinkState( retries));
    Serial.print(".");
  }
  digitalWrite( LED_BUILTIN, HIGH);
  return WiFi.status(); // return the WiFi connection status
}

void initWIFI() {
  if (initWIFIImpl() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connetted to ");
    Serial.print(WIFI_SSID);
    Serial.print(", IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Error connecting to: ");
    Serial.println(WIFI_SSID);
  }
}

void initSPIFFS() {
  bool mounted = SPIFFS.begin();
  if( mounted) {
    Serial.println("SPIFFS mounted.");
  } else {
    Serial.println("Error mounting SPIFFS");
  }
}

void initTime() {
  configTime( 1*3600, 3600, "0.pl.pool.ntp.org", "1.pl.pool.ntp.org");
  Serial.print("Waiting for time...");
  time_t now = time(nullptr);
  while( now < 1000*1000) { // default time = 28000 (1.1.1970 8:00hrs)
    Serial.print(".");
    delay(1000);
    now = time(nullptr);
  }
  Serial.print( "Time: "); Serial.println( ctime(&now));
}

void setup(void) {
  Serial.begin(115200);

  pinMode( PIN_GARAGE_DOOR, OUTPUT);
  pinMode( PIN_RAIN, INPUT);
  pinMode( LED_BUILTIN, OUTPUT);

  initSPIFFS();
  initWIFI();
  initREST();
  initTime();

  server.begin();
  Serial.println("HTTP REST Server Started");
  
  writeRainInfo();
  rainTrigger.attach( 15, writeRainInfo);

  initStatusLED();
}

void loop(void) {
  server.handleClient();
  blinkStatusLED();
}

