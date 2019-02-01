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

}
}


std::string reeToString(ree::image::ColorSpace cs) {
    return std::to_string(static_cast<int>(cs));
}