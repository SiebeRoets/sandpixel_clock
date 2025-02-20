#include "wifi_manager.hpp"

WifiMan::WifiMan(): server(80){
    
    esp_err_t err_init = nvs_flash_init();
    if (err_init == ESP_ERR_NVS_NO_FREE_PAGES || err_init == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err_init = nvs_flash_init();
    }

    if (!preferences.begin("wifi", false)) {
        Serial.println("[ERROR] Failed to initialize Preferences!");
    }
    
}
void WifiMan::initWifiBlocking(){
    String savedSSID = preferences.getString("ssid", "");
    String savedPassword = preferences.getString("password", "");
    if (savedSSID.length() > 0 && savedPassword.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
        Serial.println("Connecting to saved WiFi...");
        int time_passed=0;
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print('.');
            delay(1000);
            time_passed++;
            if(time_passed>20){
                Serial.println("WiFi connection failed. Starting AP mode...");
                WiFi.disconnect(true);  // Stop STA mode and clear connection
                initAP();
                return;
            }
        }
        if (WiFi.status() == WL_CONNECTED){
            is_connected_ =true;
        }
    }else{
        initAP();
    }
    
}
void WifiMan::handleRoot(){
    Serial.println("Sending root");
    server.send(200, "text/html", R"rawliteral(
    <html><body>
    <h2>WiFi Configuration</h2>
    <form action="/save" method="POST">
      SSID: <input type="text" name="ssid"><br>
      Password: <input type="password" name="password"><br>
      <input type="submit" value="Save and Connect">
    </form>
    </body></html>
    )rawliteral");
}
void WifiMan::handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() > 0 && password.length() > 0) {
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        server.send(200, "text/plain", "Credentials saved. Rebooting...");
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid input");
    }
}

void WifiMan::initAP(){
    WiFi.mode(WIFI_AP);
    Serial.println("Starting AP mode...");
    WiFi.softAP(apSSID, NULL);
    
    server.on("/", HTTP_GET, std::bind(&WifiMan::handleRoot, this));
    server.on("/save", HTTP_POST, std::bind(&WifiMan::handleSave, this));
    server.begin();
    Serial.println("Web server started");
    ap_active=true;
}
void WifiMan::tickWifi(uint32_t now){
    if(ap_active){
        server.handleClient();
        return;
    }
    if(now- last_tick>5000){
        last_tick =now;
        if (WiFi.status() != WL_CONNECTED){
            Serial.println("Reconnecting to WiFi...");
            WiFi.reconnect();
        }
    }
    
}

