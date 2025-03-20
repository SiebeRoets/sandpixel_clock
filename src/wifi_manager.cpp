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
void WifiMan::checkConnection(){
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        time_passed_connecting++;
        if(time_passed_connecting>2000){
            time_passed_connecting =0;
            Serial.println("WiFi connection failed. Starting AP mode...");
            WiFi.disconnect(true);  // Stop STA mode and clear connection
            initAP();
            return;
        }
    } else if (WiFi.status() == WL_CONNECTED){
        is_connected_ =true;
        Serial.println("WiFi connected!");
        current_state = WIFI_STATE::STA_CONNECTED;
        if(!ntp.is_inited){
            ntp.begin();
        }
    }
}
void WifiMan::initWifi(){
    // // DIRTY way to add brightness save
    // FONT_BRIGHTNESS = preferences.getInt("font_brightness", FONT_BRIGHTNESS); // Default to original if key doesn't exist
    // Serial.print("Pref Brightness");Serial.print(preferences.getInt("font_brightness"));Serial.println(FONT_BRIGHTNESS);
    String savedSSID = preferences.getString("ssid", "");
    String savedPassword = preferences.getString("password", "");
    if (savedSSID.length() > 0 && savedPassword.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
        Serial.println("Connecting to saved WiFi...");
        current_state = WIFI_STATE::CONNECTING_STA;
    }else{
        initAP();
    }
    server.on("/", HTTP_GET, std::bind(&WifiMan::handleRoot, this));
    server.on("/save", HTTP_POST, std::bind(&WifiMan::handleSave, this));
    server.on("/set_manual_time", HTTP_POST, std::bind(&WifiMan::handleSetTime, this));
    server.on("/set_brightness", HTTP_POST, std::bind(&WifiMan::handleSetBrightness, this));
    server.on("/reset_wifi", HTTP_POST,std::bind(&WifiMan::handleResetWifi, this));
    server.begin();
    Serial.println("Web server STA started");
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
            current_state = WIFI_STATE::CONNECTING_STA;
            is_connected_ =true;
        }
    }else{
        initAP();
    }
    
}
void WifiMan::handleRoot(){
    Serial.println("Sending root");
    server.send(200, "text/html", HTML_PAGE);
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
void WifiMan::handleSetTime() {
    if (server.hasArg("hour") && server.hasArg("minute")) {
        String hourStr = server.arg("hour");
        String minuteStr = server.arg("minute");

        uint8_t hour = hourStr.toInt();    // Convert to uint8_t
        uint8_t minute = minuteStr.toInt();

        // Validate values (hour: 0-23, minute: 0-59)
        if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
            Serial.printf("Received manual time: %02d:%02d\n", hour, minute);
            ntp.beginOffline(hour,minute);
            server.send(200, "text/plain", "Time set successfully!");
        } else {
            server.send(400, "text/plain", "Invalid time values!");
        }
    } else {
        server.send(400, "text/plain", "Missing parameters!");
    }
}
void WifiMan::handleSetBrightness() {
    // String ssid = server.arg("ssid");
    // String password = server.arg("password");
    // if (server.hasArg("brightness")) {
    //     int brightness = server.arg("brightness").toInt();
    //     Serial.printf("Received brightness: %d\n", brightness);
    //     FONT_BRIGHTNESS = brightness;
    //     preferences.putInt("font_brightness", brightness );
    //     server.send(200, "text/plain", "OK");
    // }else {
    //     server.send(400, "text/plain", "Invalid input");
    // }
}
void WifiMan::handleResetWifi() {
    preferences.remove("ssid");   
    preferences.remove("password"); 
    server.send(200, "text/plain", "WiFi credentials cleared");
    delay(1000);
    ESP.restart(); 
}
void WifiMan::initAP(){
    WiFi.mode(WIFI_AP);
    Serial.println("Starting AP mode...");
    WiFi.softAP(apSSID, NULL);
    current_state =WIFI_STATE::AP_RUNNING;
}
void WifiMan::tickWifi(uint32_t now){
    server.handleClient();
    switch(current_state){
        case WIFI_STATE::AP_RUNNING:
            return;
        case WIFI_STATE::CONNECTING_STA:
            if(now- last_tick>500){
                last_tick =now;
                checkConnection();
            }
            break;
        case WIFI_STATE::STA_CONNECTED:
            if(now- last_tick>5000){
                last_tick =now;
                if (WiFi.status() != WL_CONNECTED){
                    Serial.println("Reconnecting to WiFi...");
                    WiFi.reconnect();
                }
            }
            break;
    }
}

