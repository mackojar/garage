#include <stdio.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "credentials.h"

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

#define PIN_GARAGE_DOOR 5

ESP8266WebServer server(HTTP_REST_PORT);

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
  // check the status of WiFi connection to be WL_CONNECTED
  while ((WiFi.status() != WL_CONNECTED) /*&& (retries < MAX_WIFI_INIT_RETRY) */) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    digitalWrite( LED_BUILTIN, getBlinkState( retries));
    Serial.print(".");
  }
  digitalWrite( LED_BUILTIN, HIGH);
  return WiFi.status(); // return the WiFi connection status
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void moveDoors() {
  server.send(200, "text/html", "Moove door requested...");
  digitalWrite( PIN_GARAGE_DOOR, HIGH);
  digitalWrite( LED_BUILTIN, LOW);
  delay(500);
  digitalWrite( PIN_GARAGE_DOOR, LOW);
  digitalWrite( LED_BUILTIN, HIGH);
}

void config_rest_server_routing() {
  server.on("/api/moveDoors", HTTP_POST, []() {
    moveDoors();
  });
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });
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
    }
    else {
        Serial.print("Error connecting to: ");
        Serial.println(WIFI_SSID);
    }

    config_rest_server_routing();

    server.begin();
    Serial.println("HTTP REST Server Started");
}

void loop(void) {
    server.handleClient();
}

