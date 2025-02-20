#pragma once
#include <Arduino.h>

struct Pixel{
    uint16_t x;
    uint16_t y;
    uint8_t color;
    bool sameCoordinates(Pixel &to_compare){
        return (x == to_compare.x && y == to_compare.y);
    }
    Pixel(uint16_t x_, uint16_t y_, uint8_t color_) 
        : x(x_), y(y_), color(color_) {}
};