#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ree {
namespace image {

class ColorSpace {
public:
    enum Value {
        RGB,
        RGBA,
        Gray,
        GrayAlpha,
        YCbCr,
        Unknown,
    };

    constexpr ColorSpace(Value value) : value_(value) {}

    uint8_t Components() const {
        switch (value_) {
        case RGB:
        case YCbCr:
            return 3;
        case RGBA:
            return 4;
        case Gray:
            return 1;
        case GrayAlpha:
            return 2;
        case Unknown:
            return 0;
		default:
			return 0;
        }
    };

    std::string ToString() const {
        switch (value_) {
        case RGB:
            return "RGB";
        case YCbCr:
            return "YCbCr";
        case RGBA:
            return "RGBA";
        case Gray:
            return "Gray";
        case GrayAlpha:
            return "GrayAlpha";
        case Unknown:
            return "Unknown";
		default:
			return "Unknown";
        }
    };

    bool operator==(ColorSpace a) const { return value_ == a.value_; }
    bool operator!=(ColorSpace a) const { return value_ != a.value_; }

private:
    Value value_;
};

}
}