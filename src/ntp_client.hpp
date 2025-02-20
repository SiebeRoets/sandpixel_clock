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
        if (publicIP == "") {
            return 0; // Default to GMT 0 if IP fetch fails
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
    NTPClient(const char* server = "pool.ntp.org") {
        ntpServer = server;
    }

    void begin() {
        gmtOffset = fetchGMTOffset();  // Auto-detect GMT offset
        configTime(gmtOffset, daylightOffset, ntpServer);

        Serial.println("Waiting for NTP time sync...");
        struct tm timeinfo;
        while (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time, retrying...");
            delay(1000);
        }
        Serial.println("Time synchronized!");
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
};
