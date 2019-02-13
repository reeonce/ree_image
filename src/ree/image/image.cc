#include "image.h"

namespace ree {
namespace image {

Image::Image()
    : width(0),
      height(0),
      depthBits(0) {
}
Image::Image(int w, int h, ColorSpace cs, const std::vector<uint8_t> &&d)
    : width(w),
      height(h),
      colorspace(cs),
      data(std::move(d)) {
}

bool Image::Empty() const {
    return width == 0 && height == 0 && data.empty();
}

Image Image::ConvertToColor(ColorSpace to) const {
    if (colorspace == ColorSpace::RGBA) {
        if (to == ColorSpace::RGB) {
            std::vector<uint8_t> dst(width * height * 3);
            for (int row = 0; row < height; ++row) {
                for (int col = 0; col < width; ++col) {
                    int idx = row * width + col;
                    dst[idx * 3] = data[idx * 4];
                    dst[idx * 3 + 1] = data[idx * 4 + 1];
                    dst[idx * 3 + 2] = data[idx * 4 + 2];
                }
            }
            return Image(width, height, to, std::move(dst));
        }
    }
    return *this;
}

}
}


std::string reeToString(ree::image::ColorSpace cs) {
    return std::to_string(static_cast<int>(cs));
}