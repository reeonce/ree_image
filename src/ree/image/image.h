#pragma once

#include <cstdint>
#include <vector>

namespace ree {
namespace image {

enum class ColorSpace {
    RGB,
    RGBA,
    Gray,
    GrayAlpha,
    YCbCr
};

struct Image {
    Image();
    Image(int w, int h, ColorSpace cs, std::vector<uint8_t> &&d);

    int width;
    int height;
    ColorSpace colorspace;
    std::vector<uint8_t> data;
};

}
}