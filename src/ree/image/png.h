#pragma once

#include <ree/image/file_format_interface.h>

namespace ree {
namespace image {


/// http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
class Png : public FileFormatInterface {
public:
    std::vector<std::string> ValidExtensions() override;
    std::vector<uint8_t> MagicNumber() override;
    std::string PreferredExtension() override;

    ParseContext *CreateParseContext(ree::io::Source *source,
        const ParseOptions &options) override;
    ComposeContext *CreateComposeContext(ree::io::Source *target,
        const ComposeOptions &options) override;

    Image ParseImage(ParseContext *ctx) override;
    void ComposeImage(ComposeContext *ctx, const Image &image) override;
};

}
}
