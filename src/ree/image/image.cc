#include "image.h"

namespace ree {
namespace image {

Image::Image() {

}
Image::Image(int w, int h, ColorSpace cs, std::vector<uint8_t> &&d)
    : width(w),
      height(h),
      colorspace(cs),
      data(std::move(d)) {
}

}
}