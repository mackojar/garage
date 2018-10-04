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
    if( path.compareTo("/") == 0) {
      path = "/auth.html";
    }
    Serial.println("handleFileRead: " + path);
    String contentType = getContentType(path);
    if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      size_t sent = server.streamFile(file, contentType);
      file.close();
      return true;
    } else {
      Serial.println("\tFile Not Found");
      return false;
    }
  }
}

void moveDoors() {
  if( isAuthorized()) {
    server.sendHeader("Location", "/garage/resultOK.html", true);
    server.send(302, "text/plane","Move door requested...");
    digitalWrite( PIN_GARAGE_DOOR, HIGH);
    digitalWrite( LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite( PIN_GARAGE_DOOR, LOW);
    digitalWrite( LED_BUILTIN, HIGH);
  } else {
    server.sendHeader("Location", "/garage/resultFailed.html", true);
    server.send(302, "text/plane","Move door requested...");
  }
}

void getRainStatus() {
  if( isAuthorized()) {
    String result = "{\"rain\":";
    result.concat( digitalRead( PIN_RAIN)==LOW ? "1": "0");
    result.concat( "}");
    server.send(200, "application/json", result);
  } else {
    server.sendHeader("Location", "/auth.html");
    server.send(302);
  }
}

void initREST() {
  server.on("/api/moveDoors", HTTP_POST, []() {
    moveDoors();
  });
  server.on("/api/rainStatus", HTTP_GET, []() {
    getRainStatus();
  });
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: Not Found");
    }
  });
}
