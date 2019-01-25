#pragma once

#include <cstdint>

#include <string>
#include <vector>

#include <ree/image/image.h>

namespace ree {
namespace image {


/// http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
class PngParser {
public:
    int ParseFile(const std::string &path, Image &image);
};

}
}
