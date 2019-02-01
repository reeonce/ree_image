#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <map>

#include <ree/image/image.h>
#include <ree/image/error.h>
#include <ree/io/source.h>

namespace ree {
namespace image {
    
using ParseOptions = std::map<std::string, std::string>;
using ComposeOptions = std::map<std::string, std::string>;

class FileFormat {
public:
    /// parse the main image from source
    static int Parse(ree::io::Source *source, Image **image,
        const ParseOptions &options = ParseOptions());

    /// compose an image to target
    static int Compose(const Image &image, ree::io::Source *target,
        const ComposeOptions &options = ComposeOptions());
};

}
}