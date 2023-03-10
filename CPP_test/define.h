#pragma once
#include <iostream>

#define SafeRelease(x)              \
    if(x) {                         \
        (x)->Release();             \
        (x) = NULL;                 \
    }

#define AS_(T, ...)\
    T([&]{ T m{}; __VA_ARGS__; return m; }())

namespace SaoFU {
    struct Param {
        Param() {}

        UINT width;
        UINT height;
        UINT max_width;
        UINT max_height;

        float font_size;
        std::wstring text;
        std::wstring font_family;

        UINT pos_x;
        UINT pos_y;

        UINT blur_level;
        UINT stroke_width;

        std::wstring path;
        std::wstring name;
        GUID container_format;

        D2D1::ColorF in_solid_color_brush = 0;
        D2D1::ColorF out_solid_color_brush = 0;
    };
}