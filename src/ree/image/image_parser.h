#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <ree/image/image.h>
#include <ree/image/error.h>

namespace ree {
namespace image {

class ImageParser {
public:
virtual std::vector<std::string> ValidExtensions() = 0;
virtual std::vector<uint8_t> MagicNumber() = 0;

virtual int Parse(const std::string &path, Image &image) = 0;
virtual int Parse(const std::vector<uint8_t> &buffer, Image &image) = 0;

};

class ImageComposer {
public:
virtual std::string PreferredExtension() = 0;
virtual int Compose(const Image &image, const std::string &path) = 0;
virtual int Compose(const Image &image, std::vector<uint8_t> &buffer) = 0;
};

}
}