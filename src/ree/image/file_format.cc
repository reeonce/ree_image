#include "file_format.h"

#include <memory>

#include <ree/image/bmp.h>
#include <ree/image/png.h>
#include <ree/image/ppm.h>

namespace ree {
namespace image {

static std::shared_ptr<FileFormatInterface> FindFileFormat(
    ree::io::Source *source) {

    std::vector<std::shared_ptr<FileFormatInterface>> formats;
    formats.push_back(std::make_shared<Bmp>());
    formats.push_back(std::make_shared<Ppm>());

    std::shared_ptr<FileFormatInterface> format;

    std::vector<uint8_t> data(8);
    source->Read(data.data(), data.size());
    for (const auto &fmt: formats) {
        auto magicNumber = fmt->MagicNumber();
        bool match = true;
        for (size_t i = 0; i < magicNumber.size(); ++i) {
            if (magicNumber[i] != data[i]) {
                match = false;
                break;
            }
        }
        if (match) {
            format = fmt;
            break;
        }
    }

    source->Seek(0);
    return format;
}

int FileFormat::Parse(ree::io::Source *source, Image **image,
    const ParseOptions &options) {
    if (source->OpenToRead() != 0) {
        return ErrorCode::IOFailed;
    }

    auto format = FindFileFormat(source);
    if (!format) {
        source->Close();
        return ErrorCode::UnknownType;
    }

    auto ctx = format->CreateParseContext(source, options);
    *image = new Image(format->ParseImage(ctx));
    source->Close();
    return ctx->errCode;
}

int FileFormat::Compose(const Image &image, ree::io::Source *target,
    const ComposeOptions &options) {
    auto format = FindFileFormat(target);
    if (!format) {
        return ErrorCode::UnknownType;
    }

    if (target->OpenToWrite() != 0) {
        return ErrorCode::IOFailed;
    }

    auto ctx = format->CreateComposeContext(target, options);
    format->ComposeImage(ctx, image);
    target->Close();
    return ctx->errCode;
}

}
}
