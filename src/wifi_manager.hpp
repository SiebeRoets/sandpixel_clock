#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <nvs_flash.h>

#include <functional>
class WifiMan {
    private:
    const char *apSSID = "SieboClock_Config";
    bool ap_active{false};
    bool is_connected_{false};
    Preferences preferences;
    WebServer server;
    uint32_t last_tick{0};
    void handleRoot();
    void handleSave();
    public:
    WifiMan();
    void initAP();
    void tickWifi(uint32_t now);
    void initWifiBlocking();
    bool isConnected(){return is_connected_;};
};
