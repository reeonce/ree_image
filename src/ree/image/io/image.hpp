#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include <ree/io/source.h>
#include <ree/image/types.hpp>

namespace ree {
namespace image {
namespace io {

using LoadOptions = std::map<std::string, std::string>;
using WriteOptions = std::map<std::string, std::string>;

class Image {
public:
    /// load the main image from source
    Image Load(ree::io::Source *source,
        const LoadOptions &options = LoadOptions());

    Image(int w, int h, class ColorSpace cs, uint8_t depth,
          std::vector<uint8_t> &&d);

    int Width() const { return width_; }
    int Height() const  { return height_; }
    uint8_t DepthBits() const { return depthBits_; }
    class ColorSpace ColorSpace() const { return colorspace_; }
    const std::vector<uint8_t> &Data() const { return data_; }

    void WriteTo(ree::io::Source *target,
        const WriteOptions &options = WriteOptions()) const;

private:
    int width_;
    int height_;
    class ColorSpace colorspace_ { ColorSpace::RGBA };
    int depthBits_ { 8 };
    std::vector<uint8_t> data_;
};

}
}
}
