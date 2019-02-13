#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ree {
namespace image {

enum class ColorSpace {
    RGB,
    RGBA,
    Gray,
    GrayAlpha,
    YCbCr,
    Unknown,
};

struct Image {
    Image();
    Image(int w, int h, ColorSpace cs, const std::vector<uint8_t> &&d);

    bool Empty() const;
    Image ConvertToColor(ColorSpace to) const;

    int width;
    int height;
    int depthBits = 8;
    ColorSpace colorspace;
    std::vector<uint8_t> data;
};

}
}

std::string reeToString(ree::image::ColorSpace cs);