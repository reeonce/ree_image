#include "ppm.h"

#include <cmath>

#include <fstream>
#include <sstream>

// http://netpbm.sourceforge.net/doc/ppm.html

namespace ree {
namespace image {

std::vector<std::string> Ppm::ValidExtensions() {
    return {"ppm", "pbm", "pnm"};
}
std::vector<uint8_t> Ppm::MagicNumber() {
    return {0x50, 0x36};
}

int Ppm::Parse(const std::string &path, Image &image) {
    std::ifstream f(path, std::ifstream::binary);
    if (!f.is_open()) {
        return ErrorCode::IOFailed;
    }

    std::string header;
    header.resize(20);

    f.read(const_cast<char *>(header.data()), 20);

    if (header[0] != MagicNumber()[0] || header[1] != MagicNumber()[1]) {
        return ErrorCode::NotMatch;
    }

    std::string mc;
    int maxValue;
    int width, height, depthBits;

    std::stringstream ss(header);
    if (!(ss >> mc >> width >> height >> maxValue)) {
        return ErrorCode::FileCorrupted;
    }

    int pos = ss.tellg();
    f.seekg(pos + 1, f.beg);

    depthBits = 8;
    while ((maxValue >> depthBits) > 0) {
        depthBits++;
    }
    int bytesPerValue = depthBits > 8 ? 2 : 1;
    std::vector<uint8_t> buffer;
    buffer.resize(width * height * 3 * bytesPerValue);
    f.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    if (bytesPerValue > 1) {
        uint16_t *data = reinterpret_cast<uint16_t *>(buffer.data());
        for (int i = 0; i < width * height; ++i) {
            data[i] = (data[i] >> 8) | ((data[i] & 0xff) << 8);
        }
    }

    image = Image(width, height, ColorSpace::RGB, std::move(buffer));
    image.depthBits = depthBits;
    return ErrorCode::OK;
}
int Ppm::Parse(const std::vector<uint8_t> &buffer, Image &image) {
    return false;
}

std::string Ppm::PreferredExtension() {
    return "ppm";
}
int Ppm::Compose(const Image &image, const std::string &path) {
    std::ofstream f(path, std::ifstream::binary);
    if (!f.is_open()) {
        return ErrorCode::IOFailed;
    }

    int maxValue = (1 << image.depthBits) - 1;

    f << "P6\n";
    f << image.width << "\n";
    f << image.height << "\n";
    f << maxValue << "\n";

    if (image.depthBits <= 8) {
        f.write(reinterpret_cast<const char *>(image.data.data()),
            image.data.size());
    } else {
        std::vector<uint16_t> buffer;
        buffer.resize(image.width * image.height * 3);

        auto inData = reinterpret_cast<const uint16_t *>(image.data.data());
        for (int row = 0; row < image.height; ++row) {
            for (int col = 0; col < image.width; ++col) {
                int idx = row * image.width + col;
                buffer[idx * 3] = (inData[idx * 3] >> 8) |
                    ((inData[idx * 3] & 0xff) << 8);
                buffer[idx * 3 + 1] = (inData[idx * 3 + 1] >> 8) |
                    ((inData[idx * 3 + 1] & 0xff) << 8);
                buffer[idx * 3 + 2] = (inData[idx * 3 + 2] >> 8) |
                    ((inData[idx * 3 + 2] & 0xff) << 8);
            }
        }
        f.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * 2);
    }

    return ErrorCode::OK;
}
int Ppm::Compose(const Image &image, std::vector<uint8_t> &buffer) {
    return false;
}

}
}
