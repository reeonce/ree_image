#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ree/image/types.hpp>
#include <ree/image/io/image.hpp>

namespace ree {
namespace image {
namespace process {

/**
 * @brief normally ValueType should be uint8_t or uint16_t
 */
template <typename ValueT>
class Image {
public:
    Image(int w, int h, ColorSpace cs, uint8_t depth,
        std::vector<ValueT> &&d = std::vector<ValueT>());
    Image(int w, int h, class ColorSpace cs = ColorSpace::RGBA,
        uint8_t depth = 8)
        : Image(w, h, cs, depth) {}
    Image(class ColorSpace cs = ColorSpace::RGBA, uint8_t depth = 8)
        : Image(0, 0, cs, depth) {}

    int Width() const { return width_; }
    int Height() const  { return height_; }
    uint8_t DepthBits() const { return depthBits_; }
    class ColorSpace ColorSpace() const { return colorspace_; }
    std::vector<ValueT> &Data() { return data_; }

    void Resize(int width, int height) {
        width_ = width;
        height_ = height;
        data_.resize(width * height);
    }
    Image<ValueT> ConvertToColor(class ColorSpace to) const;

    io::Image ToIOImage() const {
        std::vector<uint8_t> data(data_.begin(), data_.end());
        return io::Image(width_, height_, colorspace_, depthBits_,
            std::move(data));
    }

private:
    int width_;
    int height_;
    class ColorSpace colorspace_ { ColorSpace::RGBA };
    int depthBits_ { 8 };
    std::vector<ValueT> data_;
};

template <typename ValueT> Image<ValueT>::Image(int w, int h,
    class ColorSpace cs, uint8_t depth, std::vector<ValueT> &&d)
    : width_(w), height_(h), colorspace_(cs), depthBits_(depth), data_(d) {
    if (w != 0 && h != 0 && data_.empty()) {
        data_.resize(w * h * cs.Components());
    }
}

template <typename ValueT>
Image<ValueT> ImageFromIOImage(const io::Image &srcImg) {
    auto &srcImgData = srcImg.Data();
    std::vector<ValueT> data(srcImgData.begin(), srcImgData.end());

    return Image<ValueT>(srcImg.Width(), srcImg.Height(), srcImg.ColorSpace(),
        srcImg.DepthBits(), std::move(data));

}

}
}
}
