#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <ree/image/io/image.hpp>
#include <ree/io/source.h>

namespace ree {
namespace image {
namespace io {

struct LoadContext {
    LoadContext(ree::io::Source *src, const LoadOptions &opt);

    ree::io::Source *source;
    LoadOptions options;

    bool done;
};

struct WriteContext {
    WriteContext(ree::io::Source *tgt, const LoadOptions &opt);
    
    ree::io::Source *target;
    WriteOptions options;
};

class FileFormat {
public:
    virtual ~FileFormat() = default;
    
    virtual std::vector<std::string> ValidExtensions() = 0;
    virtual std::string PreferredExtension() = 0;
    virtual std::vector<uint8_t> MagicNumber() = 0;

    virtual LoadContext *CreateParseContext(ree::io::Source *source,
        const LoadOptions &options) = 0;
    virtual WriteContext *CreateComposeContext(ree::io::Source *target,
        const WriteOptions &options) = 0;

    virtual Image LoadImage(LoadContext *ctx) = 0;
    virtual void WriteImage(WriteContext *ctx, const Image &image) = 0;
};

}
}
}
