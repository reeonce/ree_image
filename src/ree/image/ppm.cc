#include "ppm.h"

#include <fstream>
#include <sstream>

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

    std::stringstream ss(header);
    if (!(ss >> mc >> image.width >> image.height >> maxValue)) {
        return ErrorCode::FileCorrupted;
    }

    int pos = ss.tellg();
    f.seekg(pos + 1, f.beg);

    image.data.resize(image.width * image.height * 3);
    f.read(reinterpret_cast<char *>(image.data.data()), image.data.size());

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

    f << "P6\n";
    f << image.width << "\n";
    f << image.height << "\n";
    f << 255 << "\n";

    f.write(reinterpret_cast<const char *>(image.data.data()),
        image.data.size());

    return ErrorCode::OK;
}
int Ppm::Compose(const Image &image, std::vector<uint8_t> &buffer) {
    return false;
}

}
}
