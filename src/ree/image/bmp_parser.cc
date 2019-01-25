#include <cstdlib>
#include <cstdint>

#include <fstream>
#include <string>
#include <vector>

#include <iostream>


struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

int parse_file(const std::string &path, std::vector<uint8_t> &img);
int parse_file_header(std::ifstream &f, uint32_t &data_offset);
int parse_dib_header(std::ifstream &f, uint32_t &width, uint32_t &height,
    uint16_t &bits_per_pixel, uint32_t &data_size, uint32_t &color_palettes);
int parse_color_table_header(std::ifstream &f, 
    std::vector<Pixel> &color_palette);
int parse_color_data(std::ifstream &f, uint32_t width, uint32_t height, uint32_t linesize,
    uint16_t &bits_per_pixel, const std::vector<Pixel> &color_palette, 
    std::vector<uint8_t> &color);

int parse_file(const std::string &path, std::vector<uint8_t> &img) {
    std::ifstream f(path, std::ifstream::binary);
    if (!f.is_open()) {
        return -1;
    }
    int ret;
    uint32_t data_offset = 0;
    ret = parse_file_header(f, data_offset);
    if (ret != 0) {
        return ret;
    }

    uint32_t width, height;
    uint16_t bits_per_pixel;
    uint32_t data_size;
    uint32_t color_palettes;
    ret = parse_dib_header(f, width, height, bits_per_pixel, data_size, 
        color_palettes);
    if (ret != 0) {
        return ret;
    }
    std::vector<Pixel> color_palette;
    color_palette.resize(color_palettes);
    ret = parse_color_table_header(f, color_palette);
    if (ret != 0) {
        return ret;
    }

    uint32_t linesize = (bits_per_pixel * width + 31) / 32 * 4;
    img.resize(width * height * 4);
    f.seekg(data_offset, f.beg);
    return parse_color_data(f, width, height, linesize, bits_per_pixel, color_palette, img);
}
int parse_file_header(std::ifstream &f, uint32_t &data_offset) {
    char signature[2] = {0x00};
    f.read(signature, 2);
    if (signature[0] != 'B' || signature[1] != 'M') {
        return -2;
    }

    uint32_t file_size;
    f.read(reinterpret_cast<char *>(&file_size), 4);
    f.read(reinterpret_cast<char *>(&file_size), 4);
    f.read(reinterpret_cast<char *>(&data_offset), 4);
    return 0;
}
int parse_dib_header(std::ifstream &f, uint32_t &width, uint32_t &height,
    uint16_t &bits_per_pixel, uint32_t &data_size, uint32_t &color_palettes) {
    uint32_t dib_header_size = 0;
    f.read(reinterpret_cast<char *>(&dib_header_size), 4);
    if (dib_header_size == 40) { // BITMAPINFOHEADER
        f.read(reinterpret_cast<char *>(&width), 4);
        f.read(reinterpret_cast<char *>(&height), 4);

        uint16_t planes = 0;
        f.read(reinterpret_cast<char *>(&planes), 2);

        f.read(reinterpret_cast<char *>(&bits_per_pixel), 2);

        uint32_t compression_method = 0;
        f.read(reinterpret_cast<char *>(&compression_method), 4);

        f.read(reinterpret_cast<char *>(&data_size), 4);

        uint32_t hppm, vppm;
        f.read(reinterpret_cast<char *>(&hppm), 4);
        f.read(reinterpret_cast<char *>(&vppm), 4);

        f.read(reinterpret_cast<char *>(&color_palettes), 4);

        uint32_t useful_colors;
        f.read(reinterpret_cast<char *>(&useful_colors), 4);
    }
    return 0;
}
int parse_color_table_header(std::ifstream &f, 
    std::vector<Pixel> &color_palette) {
    f.read(reinterpret_cast<char *>(color_palette.data()), 
        4 * color_palette.size());
    return 0;
}
uint8_t read_7_bit(const uint8_t *buf, uint32_t pos) {
    uint32_t byte_index = pos / 8;
    uint8_t vl = *(buf + byte_index);

    uint32_t l_bits = 8 - pos % 8;
    if (l_bits == 8) {
        return vl >> 1;
    } else if (l_bits == 7) {
        return vl & 0x7f;
    }
    uint8_t vr = *(buf + byte_index + 1);
    uint8_t l_shift = 7 - l_bits;
    uint8_t r_shift = l_bits + 1;
    return ((vl << l_shift) | (vr >> r_shift)) & 0x7f;
}
int parse_color_data(std::ifstream &f, uint32_t width, uint32_t height, uint32_t linesize,
    uint16_t &bits_per_pixel, const std::vector<Pixel> &color_palette, 
    std::vector<uint8_t> &color) {
    std::vector<char> line;
    line.resize(linesize);
    for (uint32_t y = 0; y < height; ++y) {
        f.read(line.data(), linesize);
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t v = read_7_bit(reinterpret_cast<const uint8_t *>(line.data()), 
                bits_per_pixel * x);

            Pixel p = color_palette[v];

            std::cout << v << ": " << (int)p.r << "," << (int)p.g << "," << (int)p.b << "," << (int)p.a << std::endl;

            uint32_t idx = (y * width + x) * 4;
            color[idx + 0] = p.r;
            color[idx + 1] = p.g;
            color[idx + 2] = p.b;
            color[idx + 3] = 0xff;
        }
    }
    return 0;
}

void test_read_7_bits() {
    uint8_t buf[] = {0x34, 0x89, 0xef};
    uint8_t v = read_7_bit(buf, 0);
    v = read_7_bit(buf, 1);
    v = read_7_bit(buf, 2);
    v = read_7_bit(buf, 3);
    v = read_7_bit(buf, 4);
    v = read_7_bit(buf, 5);
    v = read_7_bit(buf, 6);
    v = read_7_bit(buf, 7);
    v = read_7_bit(buf, 8);
    v = read_7_bit(buf, 9);
    v = read_7_bit(buf, 10);
    v = read_7_bit(buf, 11);
    v = read_7_bit(buf, 12);
}

int main(int argc, char const *argv[]) {
    // test_read_7_bits();

    std::string path(argv[1]);
    std::vector<uint8_t> img;
    parse_file(path, img);

    std::ofstream out(path + ".rgba", std::ofstream::binary);
    out.write(reinterpret_cast<const char *>(img.data()), img.size());
    return 0;
}