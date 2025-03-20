#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_PixelDust.h>
#include "proj_constants.hpp"
#include "clock_render.hpp"
#include "wifi_manager.hpp"
#include <Fonts/Org_01_smaller.h>


Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();
GFXcanvas8 virtual_screen = GFXcanvas8(DISPLAY_WIDTH,DISPLAY_HEIGHT);
//IS31 does not have a queryable buffer for prev state
GFXcanvas8 matrix_buffer = GFXcanvas8(DISPLAY_WIDTH,DISPLAY_HEIGHT);

Adafruit_MPU6050 mpu = Adafruit_MPU6050();
sensors_event_t a, g, temp;
TwoWire i2c_bus = TwoWire(0);

uint8_t curr_time[5]{0,0,0,0,0};
ClockRenderer clk_render(&virtual_screen);
WifiMan wifi;


// Define previous acceleration magnitude and timestamp
float prevMagnitude = 0;
unsigned long prevTime = 0;

// Threshold values (adjust based on testing)
const float SHAKE_THRESHOLD = 6000.0;  // Minimum change in acceleration
const int SHAKE_TIME_WINDOW = 100;  // Time window in ms to confirm shaking

void drawScreen(){
  for(int8_t x = 0;x<DISPLAY_WIDTH;x++){
    for(int8_t y = 0;y<DISPLAY_HEIGHT;y++){
      if(matrix_buffer.getPixel(x,y)!=virtual_screen.getPixel(x,y)){
        // pixel differs from previous state, update
        matrix_buffer.drawPixel(x,y,virtual_screen.getPixel(x,y));
        ledmatrix.drawPixel(x,y,virtual_screen.getPixel(x,y));
      }
    }
  }

}

void setup() {
  delay(2500);
  i2c_bus.begin(2, 1, 200000);
  Serial.begin(115200);
  Serial.println("SANDDDD");
  if (! mpu.begin(0x68,&i2c_bus)) {
    Serial.println("MPU not found");
    while (1);
  }
  mpu.setGyroStandby(true,true,true);
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  if (! ledmatrix.begin(0x74,&i2c_bus)) {
    Serial.println("IS31 not found");
    while (0);
  }
  //virtual_screen.setTextColor(30);
  //virtual_screen.setRotation(1);
  ledmatrix.setRotation(2);
  virtual_screen.setFont(&Org_01);
  virtual_screen.setTextSize(1);
  virtual_screen.setTextWrap(false);
  wifi.initWifi();
  //delay(2000);

}
int load_index = 0; 
bool wifi_connecting_eff_running = false;
uint32_t now = 0;
void loop() {
  now = millis();
  wifi.tickWifi(now);
  if((now % 1000 == 0) && wifi.ntp.is_inited){
    //every second
    clk_render.setTime(wifi.ntp.getTime());
  }
  if(now % 10 == 0){
    if(wifi.current_state == WIFI_STATE::CONNECTING_STA){
      if(now % 50 == 0){
        //Draw Effect
        int fontdegrade = FONT_BRIGHTNESS;
        for(int i=0;i<LOADING_TAIL_LENGTH;i++){
          int index = load_index -i;
          if(index<0){
            index = DISPLAY_WIDTH + index;
          }
          virtual_screen.drawLine(index,0,index,DISPLAY_HEIGHT-1,fontdegrade);
          fontdegrade-=FONT_BRIGHTNESS/LOADING_TAIL_LENGTH;
          if(fontdegrade<(FONT_BRIGHTNESS/LOADING_TAIL_LENGTH)){
            fontdegrade = 0;
          }
        }
        load_index++;
        load_index = (load_index==DISPLAY_WIDTH)? 0:load_index;
        drawScreen();
        wifi_connecting_eff_running =true;
      }     
    }
    else{
        if(wifi_connecting_eff_running){
          //Wipe connecting display
          wifi_connecting_eff_running =false;
          virtual_screen.fillScreen(0);
          drawScreen();
        }
        // Read accelerometer...
        mpu.getEvent(&a, &g, &temp);
        clk_render.tick(now,-mpu.rawAccX,-mpu.rawAccY); //ADD minus when flipping screen
        drawScreen();
        float magnitude = sqrt(mpu.rawAccX*mpu.rawAccX +mpu.rawAccY*mpu.rawAccY);
        // Compute the change in acceleration
        float delta = abs(magnitude - prevMagnitude);
        // Check if change is above threshold
        if (delta > SHAKE_THRESHOLD) {
            unsigned long currentTime = millis();
            if (currentTime - prevTime < SHAKE_TIME_WINDOW) {
                Serial.println("Shake detected!");
                clk_render.fullSandify();
            }
            prevTime = currentTime; // Update last shake time
        } else if(delta>1500 && clk_render.isFullSandify()) {
          //Keep effect going when sandified
          clk_render.fullSandify();
        }
        prevMagnitude = magnitude; // Update previous magnitude
        //Serial.print("magnitude:");Serial.println(delta);
    }

  }

}















//QUICKLY SET TIME CHANGES

    // if(timer>2000){
    //   timer =0;
    //   curr_time[3]++;
    //   if(curr_time[3]==10) {curr_time[3]=0; curr_time[2]++;}
    //   if(curr_time[2]==10) curr_time[2]=0;
    //   clk_render.setTime(curr_time);
    // }




  /* Print out the values */
  // Serial.print("Acceleration X: ");
  // Serial.print(a.acceleration.x);
  // Serial.print("    ");
  // Serial.print(mpu.rawAccX);
  // Serial.print(", Y: ");
  // Serial.print(a.acceleration.y);
  // Serial.print(", Z: ");
  // Serial.print(a.acceleration.z);
  // Serial.println(" m/s^2");

  // Serial.print("Temperature: ");
  // Serial.print(temp.temperature);
  // Serial.println(" degC");