#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <ree/image/image.h>
#include <ree/image/file_format.h>
#include <ree/io/source.h>

namespace ree {
namespace image {

struct ParseContext {
    ParseContext(ree::io::Source *src, const ParseOptions &opt);

    ree::io::Source *source;
    ParseOptions options;

    ErrorCode errCode;
    bool done;
};

struct ComposeContext {
    ComposeContext(ree::io::Source *tgt, const ParseOptions &opt);
    
    ree::io::Source *target;
    ComposeOptions options;

    ErrorCode errCode;
};

class FileFormatInterface {
public:
    virtual ~FileFormatInterface();
    
    virtual std::vector<std::string> ValidExtensions() = 0;
    virtual std::string PreferredExtension() = 0;
    virtual std::vector<uint8_t> MagicNumber() = 0;

    virtual ParseContext *CreateParseContext(ree::io::Source *source,
        const ParseOptions &options) = 0;
    virtual ComposeContext *CreateComposeContext(ree::io::Source *target,
        const ComposeOptions &options) = 0;

    virtual Image ParseImage(ParseContext *ctx) = 0;
    virtual void ComposeImage(ComposeContext *ctx, const Image &image) = 0;
};

}
}
