#include "rest.h"

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { 
  if( !redirectIfNoAuth()) {
    Serial.println("handleFileRead: " + path);
    if( path.compareTo("/") == 0) {
      path += "/garage/index.html";
    }
    String contentType = getContentType(path);
    if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      size_t sent = server.streamFile(file, contentType);
      file.close();
      return true;
    }
    Serial.println("\tFile Not Found");
    return false;
  }
}

void moveDoors() {
  if( isAuthorized()) {
    server.sendHeader("Location", "/resultOK.html", true);
    server.send(302, "text/plane","Move door requested...");
    digitalWrite( PIN_GARAGE_DOOR, HIGH);
    digitalWrite( LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite( PIN_GARAGE_DOOR, LOW);
    digitalWrite( LED_BUILTIN, HIGH);
  }
}

void config_rest_server_routing() {
  server.on("/api/moveDoors", HTTP_POST, []() {
    moveDoors();
  });
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: Not Found");
    }
  });
}
