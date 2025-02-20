#include "clock_render.hpp"

void ClockRenderer::setTime(const uint8_t new_time[5]){
    virtual_screen->setTextColor(1);
    //set seconds index
    seconds_index = (new_time[4] * (DISPLAY_WIDTH-1)) / 60;
    //seconds between two ticks of this function is to big, it means seconde bar should redraw
    if(time_save[4]-new_time[4]>2) run_effect_seconds_bar=0;  
    
    if(full_sandify && new_time[0]!=10){
        //save new time while sandify is ongoing
        for(int i=0; i<5; i++) {
            time_save[i]= new_time[i];
        }
        return;
    }
    bool any_changed{false};
    for(int i=0; i<4; i++) {
        if(time[i] != new_time[i]){
            any_changed = true;
            digit_changed[i] = true;
        }else{
            digit_changed[i] = false;
        }
    }
    
    if(!any_changed) return;
    sand_pixels.clear();
    //check hours minutes
    for(int i=0; i<4; i++) {
        if(digit_changed[i]){
            time[i] = new_time[i];
            uint8_t start_x = NUM_INDEX[i]-1;
            //Check what pixels are lit
            for(int8_t x =start_x;x<start_x+3 ;x++){
                for(int8_t y = 0;y<DISPLAY_HEIGHT-1;y++){
                    if(virtual_screen->getPixel(x,y)==FONT_BRIGHTNESS){
                        // pixel is lit from previous number, clear
                        virtual_screen->drawPixel(x,y,0);
                        // Save for pixel sand simulation
                        sand_pixels.emplace_back(x,y,FONT_BRIGHTNESS);
                    }
                }
            }
            //Don't set time for full sendify (=10)
            if(time[i]==10) continue;
            time_save[i]=time[i];
            time_save[4]= new_time[4];
            virtual_screen->setCursor(start_x,4);
            virtual_screen->print(time[i]);
            Serial.print("Time check is ");Serial.println(new_time[i]);
            //Check what pixels are lit
            for(int8_t x =start_x;x<start_x+3 ;x++){
                for(int8_t y = 0;y<DISPLAY_HEIGHT-1;y++){
                    if(virtual_screen->getPixel(x,y)==1){
                        // pixel is lit
                        //Serial.println("pixel is added to vector");
                        pixels_to_change.emplace_back(x,y,0);
                        virtual_screen->drawPixel(x,y,0);
                    }
                }
            }            
        }
    }
    if(!sand_pixels.empty()){
        //start sand simulation
        sand.setGrains(sand_pixels);
        sand.createNewSimulation(sand_pixels.size(),digit_changed);
    }

}
void ClockRenderer::tick(uint32_t now,int16_t acc_x, int16_t acc_y){
    if(!pixels_to_change.empty()){
        // Seed random number generator
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        // Select one random index
        size_t index = std::rand() % pixels_to_change.size();
        if(sand.isRunning() && pixels_to_change[index].color == 0){
            //set obstacle for sand first
            sand.setExtraObstacle(pixels_to_change[index].x,pixels_to_change[index].y);
        }
        // Increment the color of the selected pixel
        pixels_to_change[index].color +=5;
        
        // If the color exceeds BRIGHTNESS, set it to BRIGHTNESS and remove the pixel
        if (pixels_to_change[index].color > FONT_BRIGHTNESS) {
            pixels_to_change[index].color = FONT_BRIGHTNESS;
            virtual_screen->drawPixel(pixels_to_change[index].x,pixels_to_change[index].y,pixels_to_change[index].color);
            pixels_to_change.erase(pixels_to_change.begin() + index);
        }
        else{
            virtual_screen->drawPixel(pixels_to_change[index].x,pixels_to_change[index].y,pixels_to_change[index].color);
        }
    }else{
        //if list is empty we should start rerendering the clock at some point
        // if(full_sandify&& (full_sandify_time_passed > (FULL_SANDIFY_TIME-SAND_FADOUT_TIME))){
        //     //start rendering digits  of saved time
        //     this->setTime(time_save);
        // }
    }
    sand.tick(now,acc_x,acc_y);
    if(full_sandify){
        full_sandify_time_passed+=10;
        if ((full_sandify_time_passed > FULL_SANDIFY_TIME)){
            //stop full sandify
            full_sandify = false;
            Serial.println("Full sendify stopped");
            full_sandify_time_passed = 0;
            //reset timeout for regular time updates
            sand.setSandDimTimeout(SAND_FADOUT_TIME);
            //restore screen
            this->setTime(time_save);
            
        }
    }
    sec_brightness+=seconds_brightness_inc;
    if(sec_brightness>FONT_BRIGHTNESS){
        sec_brightness= FONT_BRIGHTNESS;
        //reset brightness this when index is moved to next pixel
        if(virtual_screen->getPixel(seconds_index,DISPLAY_HEIGHT-1)==0){
            sec_brightness = 0;
        }
    }
    // if no sand sim is running
    if(!sand.isRunning()){
        //After effect refill seconds bar
        if(run_effect_seconds_bar>=0){
            virtual_screen->drawPixel(run_effect_seconds_bar/10,DISPLAY_HEIGHT-1,FONT_BRIGHTNESS);
            run_effect_seconds_bar++;
            //set to -1 to stop this fill effect
            //Serial.print("Sec bar eff is ");Serial.println((int)run_effect_seconds_bar/10);
            run_effect_seconds_bar = (run_effect_seconds_bar/10==seconds_index)? -1: run_effect_seconds_bar;
        }else{
            //draw line untill seconds index
            for(int i=0; i<DISPLAY_WIDTH; i++) {
                if(i<seconds_index) virtual_screen->drawPixel(i,DISPLAY_HEIGHT-1,FONT_BRIGHTNESS);
                if(i == seconds_index) virtual_screen->drawPixel(i,DISPLAY_HEIGHT-1,sec_brightness);
                if(i > seconds_index) virtual_screen->drawPixel(i,DISPLAY_HEIGHT-1,0);
            }
        }
        
    }
}
   
void ClockRenderer::fullSandify(){
    if(full_sandify || !pixels_to_change.empty()) return;
    full_sandify =true;
    Serial.println("Full sendify started");
    sand.setSandDimTimeout(FULL_SANDIFY_TIME);
    uint8_t timeArray[5] = {10, 10, 10, 10,0};
    this->setTime(timeArray);
}


// FADE IN/OUT effect

    // sec_brightness+=seconds_brightness_inc;
    // if(sec_brightness>FONT_BRIGHTNESS){
    //     seconds_brightness_inc*=-1;
    //     sec_brightness= FONT_BRIGHTNESS;
    // }else if(sec_brightness<0){
    //     sec_brightness=0;
    //     seconds_brightness_inc*=-1;
    // }
