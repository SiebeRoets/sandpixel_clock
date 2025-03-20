#include <Arduino.h>
#include <memory> 
#include <Adafruit_GFX.h>
#include <Adafruit_PixelDust.h>
#include "proj_constants.hpp"
#include "proj_structs.hpp"

class SandRenderer {
    private:
    GFXcanvas8* virtual_screen;
    grain_count_t n_grains;
    std::unique_ptr<Adafruit_PixelDust> pixel_dust;
    std::vector<Pixel> prev_grain;
    std::vector<Pixel> new_pixels;
    bool is_running{false};
    u_int8_t sand_brightness{FONT_BRIGHTNESS};
    uint32_t last_tick{0};
    uint32_t last_dim_tick{0};
    const uint32_t sand_timeout {1000/SAND_FPS};
    uint32_t sand_dim_timeout {SAND_FADOUT_TIME/FONT_BRIGHTNESS};
    public:
    SandRenderer(GFXcanvas8* display_pointer):virtual_screen(display_pointer){};
    void createNewSimulation(uint16_t n_grains,bool (&changing_digits)[4]);
    //setGrains MUST be called before createNewSimulation
    void setGrains(const std::vector<Pixel>& pixels);
    void setExtraObstacle(uint16_t x,uint16_t y);
    bool isRunning(){return is_running;};
    bool checkObstacleCollision(uint16_t x,uint16_t y);
    void stopSimulation();
    void setSandDimTimeout(uint32_t fadeout_time);
    void tick(uint32_t now, int16_t acc_x, int16_t acc_y);
    void resetSandBrightness(){sand_brightness = FONT_BRIGHTNESS-1;};
};