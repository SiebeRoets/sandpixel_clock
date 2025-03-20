#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <functional>
#include "ntp_client.hpp"
#include "html_page.hpp"
#include "proj_constants.hpp"
enum class WIFI_STATE {
    NONE = 0,
    CONNECTING_STA,
    STA_CONNECTED,
    AP_RUNNING
};
class WifiMan {
    private:
    const char *apSSID = "SieboClock_Config";
    int time_passed_connecting {0};
    bool is_connected_{false};
    
    Preferences preferences;
    WebServer server;
    uint32_t last_tick{0};
    void handleRoot();
    void handleSave();
    void handleSetTime();
    void handleSetBrightness();
    void handleResetWifi();
    public:
    WIFI_STATE current_state{WIFI_STATE::NONE}; 
    NTPClient ntp;  // Auto-detects timezone
    WifiMan();
    void initAP();
    void tickWifi(uint32_t now);
    void checkConnection();
    void initWifi();
    void initWifiBlocking();
    bool isConnected(){return is_connected_;};
};
