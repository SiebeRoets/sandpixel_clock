#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

class NTPClient {
private:
    const char* ntpServer;
    int gmtOffset = 0;  // Auto-detected later
    int daylightOffset = 0;
    uint8_t time_array [5];
    String getPublicIP() {
        HTTPClient http;
        http.begin("http://api.ipify.org");  // Get public IP
        int httpCode = http.GET();

        if (httpCode == 200) {
            String ip = http.getString();
            http.end();
            Serial.println("Public IP: " + ip);
            return ip;
        } else {
            Serial.println("Failed to get public IP!");
            http.end();
            return "";
        }
    }

    int fetchGMTOffset() {
        String publicIP = getPublicIP();
        while(publicIP == "") {
            delay(1000);
            Serial.println("Failed to get public ip, retrying");
            getPublicIP(); 
        }

        String url = "https://www.timeapi.io/api/TimeZone/ip?ipAddress=" + publicIP;
        HTTPClient http;
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode == 200) {
            String payload = http.getString();
            http.end();

            // Parse JSON response
            StaticJsonDocument<1024> doc;
            deserializeJson(doc, payload);

            int offsetSeconds = doc["currentUtcOffset"]["seconds"];
            Serial.printf("Detected GMT Offset: %d seconds\n", offsetSeconds);
            return offsetSeconds;
        } else {
            Serial.println("Failed to get timezone, defaulting to GMT 0.");
            http.end();
            return 0;
        }
    }

public:
    bool is_inited{false};
    NTPClient(const char* server = "pool.ntp.org") {
        ntpServer = server;
    }

    void begin() {
        gmtOffset = fetchGMTOffset();  // Auto-detect GMT offset
        configTime(gmtOffset, daylightOffset, ntpServer);

        Serial.println("Waiting for NTP time sync...");
        struct tm timeinfo;
        int retries = 10;  // Try 10 times before giving up
        while (!getLocalTime(&timeinfo) && retries > 0) {
            Serial.println("Failed to obtain time, retrying...");
            delay(1000);
            retries--;
        }
        if(retries ==0){
            Serial.println("NTP sync failed, set manual time...");
            return;
        }
        Serial.println("Time synchronized!");
        is_inited = true;
    }
    void beginOffline(uint8_t hour,uint8_t minute){
        configTime(gmtOffset, daylightOffset, ntpServer);
        setManualTime(2025,1,1,hour,minute,0);
        is_inited = true;
    }
    uint8_t* getTime() {
        struct tm timeinfo;

        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to get time!");
            memset(time_array, 0, sizeof(time_array));
            return time_array;
        }
        time_array[0] = timeinfo.tm_hour / 10;  
        time_array[1] = timeinfo.tm_hour % 10;  
        time_array[2] = timeinfo.tm_min / 10;    
        time_array[3] = timeinfo.tm_min % 10;    
        time_array[4] = timeinfo.tm_sec;   
        return time_array;
    }
    void setManualTime(int year, int month, int day, int hour, int minute, int second) {
        struct tm manualTime = {};
        manualTime.tm_year = year - 1900;
        manualTime.tm_mon = month - 1;
        manualTime.tm_mday = day;
        manualTime.tm_hour = hour;
        manualTime.tm_min = minute;
        manualTime.tm_sec = second;

        time_t t = mktime(&manualTime);
        struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
        settimeofday(&tv, NULL);

        Serial.println("Manual time set successfully.");
    }
};

