#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "sand_render.hpp"
#include "proj_constants.hpp"
#include "proj_structs.hpp"

#include <vector>
#include <cstdlib>  
#include <ctime>    

class ClockRenderer {
    private:
    SandRenderer sand;
    GFXcanvas8* virtual_screen;
    uint8_t time[4]{10,10,10,10};
    uint8_t time_save[5]{10,10,10,10,0};
    uint8_t seconds_index {0};
    bool digit_changed[4]{false,false,false,false};
    std::vector<Pixel> pixels_to_change;
    std::vector<Pixel> sand_pixels;
    bool updating {false};
    bool full_sandify {false};
    int run_effect_seconds_bar {-1};
    uint32_t full_sandify_time_passed {0};
    uint32_t last_tick{0};
    float sec_brightness{0};
    float seconds_brightness_inc {((float)FONT_BRIGHTNESS)/(6000.0f/ (float)DISPLAY_WIDTH)}; //0-100% should map to exactly how many seconds the second tick stays at that pixel 
    public:
    ClockRenderer(GFXcanvas8* display_pointer):virtual_screen(display_pointer),sand(display_pointer){
        
    };
    void setTime(const uint8_t new_time[5]);
    void fullSandify();
    void tick(uint32_t now,int16_t acc_x, int16_t acc_y);
};