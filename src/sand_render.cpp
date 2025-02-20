#include "sand_render.hpp"
#include <vector>

void SandRenderer::createNewSimulation(uint16_t n_grains,bool (&changing_digits)[4]){

    pixel_dust = std::make_unique<Adafruit_PixelDust>(DISPLAY_WIDTH, DISPLAY_HEIGHT, n_grains, 1, 180);
    if(!pixel_dust->begin())  Serial.println("Sand could not be made");
    if(!prev_grain.empty()){
        for (int i = 0; i< prev_grain.size();i++) {
            pixel_dust->setPosition(i,prev_grain[i].x,prev_grain[i].y);
        }
    }
    for (int i = 0; i< 4;i++) {
        if(!changing_digits[i]){
            //set obstacle
            uint16_t start_x = NUM_INDEX[i]-1;
            for(uint16_t x =start_x;x<start_x+3 ;x++){
                for(uint16_t y = 0;y<DISPLAY_HEIGHT-1;y++){
                    if(virtual_screen->getPixel(x,y)==FONT_BRIGHTNESS){
                        // pixel is lit
                        pixel_dust->setPixel(x,y);
                    }
                }
            }
        }
    }
    is_running=true;
}
void SandRenderer::setExtraObstacle(uint16_t x,uint16_t y){
    new_pixels.emplace_back(x,y,0);
}
bool SandRenderer::checkObstacleCollision(uint16_t x,uint16_t y){
    for (const auto& pixel : new_pixels) {
        if(pixel.x == x && pixel.y == y) return true;
    }
    return false;
}
void SandRenderer::setGrains(const std::vector<Pixel>& pixels) {
        if(pixel_dust){
            stopSimulation();
        }
        prev_grain = pixels; 
    }
void SandRenderer::stopSimulation(){
    for (int i = 0; i< prev_grain.size();i++) {
        //clear grains from screen
        if(!checkObstacleCollision(prev_grain[i].x,prev_grain[i].y)){
        virtual_screen->drawPixel(prev_grain[i].x,prev_grain[i].y,0);
        }
    }
    sand_brightness = FONT_BRIGHTNESS-1;
    pixel_dust.reset();
    prev_grain.clear();
    new_pixels.clear();
    is_running = false;
}
void SandRenderer::setSandDimTimeout(uint32_t fadeout_time){
    sand_dim_timeout= fadeout_time/FONT_BRIGHTNESS;
}
void SandRenderer::tick(uint32_t now,int16_t acc_x, int16_t acc_y){
    if(!pixel_dust) return;
    if((now - last_dim_tick) >sand_dim_timeout){
        sand_brightness--;
        if(sand_brightness == 0){
            //end of simulation
            stopSimulation();
            return;
        }
        last_dim_tick = now;
    }
    if((now - last_tick) <sand_timeout){
        return;
    }
    last_tick = now;
    // Run one frame of the simulation
    pixel_dust->iterate(acc_y,acc_x);
    // Check updated coordinates
    dimension_t x, y;
    for (int i = 0; i< prev_grain.size();i++) {
        
        pixel_dust->getPosition(i,&x,&y);
        if((prev_grain[i].x != x || prev_grain[i].y != y)){
            if(!checkObstacleCollision(prev_grain[i].x,prev_grain[i].y)){
                virtual_screen->drawPixel(prev_grain[i].x,prev_grain[i].y,0);
            }
        }
        if(!checkObstacleCollision(x,y)){
                virtual_screen->drawPixel(x,y,sand_brightness);
        }
        prev_grain[i].x = x;
        prev_grain[i].y = y;
    }
    return;
}












// void SandRenderer::sandifyNumber(uint8_t index)
// {
//         std::vector<Grain> number_grains;
//         std::vector<Grain> running_grains;
//         // no sand is currently running
//         for (int8_t x = NUM_INDEX[index]; x < NUM_INDEX[index]+FONT_WIDTH; x++){
//             for (int8_t y = 0; y < DISPLAY_HEIGHT; y++){
//                 //this is the number we will sandify
//                     if (virtual_screen->getPixel(x, y) == FONT_BRIGHTNESS){
//                         //save to set later
//                         Grain g1 = {x, y, 100, 100};
//                         number_grains.push_back(g1);
//                         virtual_screen->drawPixel(x, y,FONT_BRIGHTNESS - 1);
//                     }
//             }
//         }
//         if (sand != nullptr){
//             //Sand is currently running, save sand for next simulation
//             for (int16_t i= 0; i < n_grains; i++){
//                 Grain* original = sand->getGrain(i);
//                 Grain g1 = {original->x, original->y, original->vx, original->vy};
//                 running_grains.push_back(g1);
//             }
//             //delete current simulation
//             delete sand;
//         }
//         n_grains+=number_grains.size();
//         sand = new Adafruit_PixelDust(DISPLAY_WIDTH, DISPLAY_HEIGHT, n_grains, 1, 180);
//         if(!sand->begin())  Serial.println("Sand could not be made");
//         for (int8_t x = 0; x < DISPLAY_WIDTH; x++){
//             for (int8_t y = 0; y < DISPLAY_HEIGHT; y++){
//                 if (virtual_screen->getPixel(x, y) == FONT_BRIGHTNESS){
//                         //Number should become obstacle
//                         sand->setPixel(x,y);
//                 }
//             }
//         }
//         int16_t grain_index = 0;
//         if(running_grains.size()>0){
//             //Place grains from previous simulation first
//             for (grain_index; grain_index < running_grains.size(); ++grain_index) {
//                 sand->setPosition(grain_index,running_grains[grain_index].x,running_grains[grain_index].y);
//                 sand->getGrain(grain_index)->vx = running_grains[grain_index].vx;
//                 sand->getGrain(grain_index)->vy = running_grains[grain_index].vy;
//             }
//         }
//         //Place grains from number
//         for (grain_index; grain_index < n_grains; ++grain_index) {
//             sand->setPosition(grain_index,number_grains[grain_index].x,number_grains[grain_index].y);
//         }
//         return;
// }