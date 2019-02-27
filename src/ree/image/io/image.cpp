#include "image.hpp"

#include <ree/image/io/file_format.hpp>
#include <ree/image/io/jpeg.hpp>
#include <ree/image/io/png.hpp>
#include <ree/image/io/bmp.hpp>
#include <ree/image/io/ppm.hpp>
#include <ree/image/io/error.hpp>

namespace ree {
namespace image {
namespace io {

static std::shared_ptr<FileFormat> FindFileFormat(ree::io::Source *source) {
    std::vector<std::shared_ptr<FileFormat>> formats = {
        std::make_shared<Jpeg>(),
        std::make_shared<Png>(),
        std::make_shared<Bmp>(),
        std::make_shared<Ppm>(),
    };

    std::shared_ptr<FileFormat> format;

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

Image Image::Load(ree::io::Source *source, const LoadOptions &options) {
    source->OpenToRead();

    auto format = FindFileFormat(source);
    if (!format) {
        source->Close();
        throw UnknownFormatException();
    }

    auto ctx = format->CreateParseContext(source, options);
    auto image = format->LoadImage(ctx);
    source->Close();
    return image;
}

Image::Image(int w, int h, class ColorSpace cs, uint8_t depth, 
    std::vector<uint8_t> &&d)
    : width_(w), height_(h), colorspace_(cs), depthBits_(depth), data_(d) {
    if (w != 0 && h != 0 && data_.empty()) {
        data_.resize(w * h * cs.Components());
    }
}

void Image::WriteTo(ree::io::Source *target, const WriteOptions &options) const {
    auto format = FindFileFormat(target);
    if (!format) {
        throw UnknownFormatException();
    }

    target->OpenToWrite();
    auto ctx = format->CreateComposeContext(target, options);
    format->WriteImage(ctx, *this);
    target->Close();
}

}
}
}