#include "image.h"

namespace ree {
namespace image {

Image::Image() {

}
Image::Image(int w, int h, ColorSpace cs, const std::vector<uint8_t> &&d)
    : width(w),
      height(h),
      colorspace(cs),
      data(std::move(d)) {
}

}
}


std::string reeToString(ree::image::ColorSpace cs) {
    return std::to_string(static_cast<int>(cs));
}