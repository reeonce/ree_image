#pragma once

#include <ree/image/io/file_format.hpp>

namespace ree {
namespace image {
namespace io {

class Jpeg : public FileFormat {
public:
    std::vector<std::string> ValidExtensions() override;
    std::vector<uint8_t> MagicNumber() override;
    std::string PreferredExtension() override;

    LoadContext *CreateParseContext(ree::io::Source *source,
        const LoadOptions &options) override;
    WriteContext *CreateComposeContext(ree::io::Source *target,
        const WriteOptions &options) override;

    Image LoadImage(LoadContext *ctx) override;
    void WriteImage(WriteContext *ctx, const Image &image) override;
};

}
}
}
