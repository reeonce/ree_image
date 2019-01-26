#pragma once

#include <ree/image/image_parser.h>

namespace ree {
namespace image {

class Bmp : public ImageParser, public ImageComposer {
public:
std::vector<std::string> ValidExtensions() override;
std::vector<uint8_t> MagicNumber() override;

int Parse(const std::string &path, Image &image) override;
int Parse(const std::vector<uint8_t> &buffer, Image &image) override;

std::string PreferredExtension() override;
int Compose(const Image &image, const std::string &path) override;
int Compose(const Image &image, std::vector<uint8_t> &buffer) override;

};

}
}