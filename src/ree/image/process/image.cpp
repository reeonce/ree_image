#include "image.hpp"

namespace ree {
namespace image {
namespace process {



template <typename ValueT> Image<ValueT>::Image(int w, int h,
    class ColorSpace cs, uint8_t depth, std::vector<ValueT> &&d)
    : width_(w), height_(h), colorspace_(cs), depthBits_(depth), data_(d) {
    if (w != 0 && h != 0 && data_.empty()) {
        data_.resize(w * h * cs.Components());
    }
}

template <typename ValueT>
Image<ValueT> Image<ValueT>::ConvertToColor(class ColorSpace to) const {
    Image<ValueT> dst(width_, height_, to, depthBits_, std::vector<ValueT>());

    if (colorspace_ == ColorSpace::RGBA) {
        if (to == ColorSpace::RGB) {
            for (int row = 0; row < height_; ++row) {
                for (int col = 0; col < width_; ++col) {
                    int idx = row * width_ + col;
                    dst.data_[idx * 3] = data_[idx * 4];
                    dst.data_[idx * 3 + 1] = data_[idx * 4 + 1];
                    dst.data_[idx * 3 + 2] = data_[idx * 4 + 2];
                }
            }
        }
    }
    return dst;
}


template <typename ValueT>
Image<ValueT> ImageFromIOImage(const io::Image &srcImg) {
    auto &srcImgData = srcImg.Data();
    std::vector<ValueT> data(srcImgData.begin(), srcImgData.end());

    return Image<ValueT>(srcImg.Width(), srcImg.Height(), srcImg.ColorSpace(),
        srcImg.DepthBits(), std::move(data));

}



template Image<uint8_t>::Image(int w, int h, class ColorSpace cs,
    uint8_t depth, std::vector<uint8_t> &&d);
template Image<uint16_t>::Image(int w, int h, class ColorSpace cs,
    uint8_t depth, std::vector<uint16_t> &&d);

template Image<uint8_t> Image<uint8_t>::ConvertToColor(class ColorSpace to) const;
template Image<uint16_t> Image<uint16_t>::ConvertToColor(class ColorSpace to) const;

template Image<uint8_t> ImageFromIOImage<uint8_t>(const io::Image &srcImg);
template Image<uint16_t> ImageFromIOImage<uint16_t>(const io::Image &srcImg);

}
}
}