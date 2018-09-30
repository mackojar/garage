typedef struct TimeData {
    char filename[30];
    char timestamp[10];
};

struct TimeData * getFileName(struct TimeData *timeData) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    sprintf(timeData->timestamp, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    sprintf(timeData->filename, "/logs/rain%02d%02d%02d.csv", timeinfo->tm_year % 100, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    //Serial.printf("File: %s Timestamp: %s\n", timeData->filename, timeData->timestamp);
    return timeData;
}

// read dir logs and remove old files
void removeOldFiles() {
    // count files
    int count = 0;
    Dir dir = SPIFFS.openDir("/logs");
    while( dir.next()) {
        count++;
    }
    Serial.print("Log files count: "); Serial.println(count);
    if( count > 3) {
        dir = SPIFFS.openDir("/logs");
        while( dir.next() && count > 3)  {
            Serial.print( "Removing old log file: ");
            Serial.println( dir.fileName());
            SPIFFS.remove( dir.fileName());
            count--;
        }
    }
}

void writeRainInfo() {
    struct TimeData timeData;
    getFileName(&timeData);
    File f = SPIFFS.open(timeData.filename, "a");
    if( !f) {
        Serial.print("Can't open status file: ");
        Serial.println(timeData.filename);
    } else {
        int isRain = !digitalRead(PIN_RAIN);
        const char* rainInfo = isRain ? "wet" : "dry";
        Serial.printf("File: %s Timestamp: %s Rain: %s\n", timeData.filename, timeData.timestamp, rainInfo);
        f.printf("%s: %s\n", timeData.timestamp, rainInfo);
        f.close();
    }
    removeOldFiles();
}
