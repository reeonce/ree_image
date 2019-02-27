#include "ppm.hpp"

#include <cmath>
#include <sstream>

#include <ree/image/io/error.hpp>

// http://netpbm.sourceforge.net/doc/ppm.html
namespace ree {
namespace image {
namespace io {

std::vector<std::string> Ppm::ValidExtensions() {
    return {"ppm", "pbm", "pnm"};
}
std::vector<uint8_t> Ppm::MagicNumber() {
    return {0x50, 0x36};
}
std::string Ppm::PreferredExtension() {
    return "ppm";
}

LoadContext *Ppm::CreateParseContext(ree::io::Source *source,
    const LoadOptions &options) {
    return new LoadContext(source, options);
}
WriteContext *Ppm::CreateComposeContext(ree::io::Source *target,
    const WriteOptions &options) {
    return new WriteContext(target, options);
}

Image Ppm::LoadImage(LoadContext *ctx) {
    auto source = ctx->source;

    std::string header;
    header.resize(20);

    auto headerBuffer = reinterpret_cast<const uint8_t *>(header.data());
    source->Read(const_cast<uint8_t *>(headerBuffer), header.size());

    auto magicNumber = this->MagicNumber();
    if (header[0] != magicNumber[0] || header[1] != magicNumber[1]) {
		throw FileCorruptedException("Magic number not match.");
    }

    std::string mc;
    int maxValue;
    int width, height, depthBits;

    std::stringstream ss(header);
    if (!(ss >> mc >> width >> height >> maxValue)) {
		throw FileCorruptedException("wrong header.");
    }

    size_t pos = ss.tellg();
    source->Seek(pos + 1);

    depthBits = 8;
    while ((maxValue >> depthBits) > 0) {
        depthBits++;
    }
    int bytesPerValue = depthBits > 8 ? 2 : 1;
    std::vector<uint8_t> buffer;
    buffer.resize(width * height * 3 * bytesPerValue);
    source->Read(buffer.data(), buffer.size());

    if (bytesPerValue > 1) {
        uint16_t *data = reinterpret_cast<uint16_t *>(buffer.data());
        for (int i = 0; i < width * height; ++i) {
            data[i] = (data[i] >> 8) | ((data[i] & 0xff) << 8);
        }
    }

    Image image(width, height, ColorSpace::RGB, depthBits, std::move(buffer));
    return image;
}
void Ppm::WriteImage(WriteContext *ctx, const Image &image) {
    auto target = ctx->target;

    int maxValue = (1 << image.DepthBits()) - 1;

    std::stringstream ss;
    ss << "P6\n";
    ss << image.Width() << "\n";
    ss << image.Height() << "\n";
    ss << maxValue << "\n";

    std::string header = ss.str();
    target->Write(reinterpret_cast<const uint8_t *>(header.data()),
        header.size());

    if (image.DepthBits() <= 8) {
        target->Write(image.Data().data(), image.Data().size());
    } else {
        std::vector<uint16_t> buffer;
        buffer.resize(image.Width() * image.Height() * 3);

        auto inData = reinterpret_cast<const uint16_t *>(image.Data().data());
        for (int row = 0; row < image.Height(); ++row) {
            for (int col = 0; col < image.Width(); ++col) {
                int idx = row * image.Width() + col;
                buffer[idx * 3] = (inData[idx * 3] >> 8) |
                    ((inData[idx * 3] & 0xff) << 8);
                buffer[idx * 3 + 1] = (inData[idx * 3 + 1] >> 8) |
                    ((inData[idx * 3 + 1] & 0xff) << 8);
                buffer[idx * 3 + 2] = (inData[idx * 3 + 2] >> 8) |
                    ((inData[idx * 3 + 2] & 0xff) << 8);
            }
        }
        target->Write(reinterpret_cast<const uint8_t *>(buffer.data()),
            buffer.size() * 2);
    }
}

}
}
}