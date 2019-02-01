#include "bmp.h"

#include <iostream>

#include <ree/io/bit_buffer.h>

namespace ree {
namespace image {

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

enum DIBHeaderType {
    BITMAPCOREHEADER = 12,
    OS22XBITMAPHEADERS = 16,
    BITMAPINFOHEADER = 40,
    BITMAPV2INFOHEADER = 52,
    BITMAPV3INFOHEADER = 56,
    OS22XBITMAPHEADER = 64,
    BITMAPV4HEADER = 108,
    BITMAPV5HEADER = 124,
};

struct BmpContext : public ParseContext {
    using ParseContext::ParseContext;

    uint32_t data_offset;
    uint32_t file_size;

    DIBHeaderType dib_header_type;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression_method;
    uint32_t data_size;
    uint32_t hppm;
    uint32_t vppm;
    uint32_t color_palettes;
    uint32_t useful_colors;

    std::vector<Pixel> color_palette;

    std::vector<uint8_t> color;
};

static int parse_file_header(ree::io::Source &source, BmpContext &ctx);
static int parse_dib_header(ree::io::Source &source, BmpContext &ctx);
static int parse_color_table_header(ree::io::Source &source, BmpContext &ctx);
static int parse_color_data(ree::io::Source &source, BmpContext &ctx);


std::vector<std::string> Bmp::ValidExtensions() {
    return { "bmp", "BMP"};
}
std::vector<uint8_t> Bmp::MagicNumber() {
    return {'B', 'M'};
}
std::string Bmp::PreferredExtension() {
    return "bmp";
}

ParseContext *Bmp::CreateParseContext(ree::io::Source *source,
    const ParseOptions &options) {
    return new BmpContext(source, options);
}
ComposeContext *Bmp::CreateComposeContext(ree::io::Source *target,
    const ComposeOptions &options) {
    return nullptr;
}

Image Bmp::ParseImage(ParseContext *contex) {
    auto ctx = reinterpret_cast<BmpContext *>(contex);
    auto source = ctx->source;

    if (source->OpenToRead() != 0) {
        ctx->errCode = ErrorCode::IOFailed;
        return Image();
    }

    if (parse_file_header(*source, *ctx) != 0) {
        ctx->errCode = ErrorCode::FileCorrupted;
        return Image();
    }
    if (parse_dib_header(*source, *ctx) != 0) {
        ctx->errCode = ErrorCode::FileCorrupted;
        return Image();
    }
    if (parse_color_table_header(*source, *ctx) != 0) {
        ctx->errCode = ErrorCode::FileCorrupted;
        return Image();
    }
    source->Seek(ctx->data_offset);
    if (parse_color_data(*source, *ctx) != 0) {
        ctx->errCode = ErrorCode::FileCorrupted;
        return Image();
    }
    return Image(ctx->width, ctx->height, ColorSpace::RGB, std::move(ctx->color));
}
void Bmp::ComposeImage(ComposeContext *ctx, const Image &image) {
    ctx->errCode = ErrorCode::NotImplemented;
}

int parse_file_header(ree::io::Source &source, BmpContext &ctx) {
    uint8_t signature[2] = {0x00};
    source.Read(signature, sizeof(signature));
    if (signature[0] != 'B' || signature[1] != 'M') {
        return -2;
    }

    source.Read(reinterpret_cast<uint8_t *>(&ctx.file_size), 4);
    uint8_t reserved[4];
    source.Read(reserved, sizeof(reserved));
    source.Read(reinterpret_cast<uint8_t *>(&ctx.data_offset), 4);
    return 0;
}
int parse_dib_header(ree::io::Source &source, BmpContext &ctx) {
    source.Read(reinterpret_cast<uint8_t *>(&ctx.dib_header_type), 4);
    if (ctx.dib_header_type == BITMAPINFOHEADER) { // BITMAPINFOHEADER
        source.Read(reinterpret_cast<uint8_t *>(&ctx.width), 4);
        source.Read(reinterpret_cast<uint8_t *>(&ctx.height), 4);

        source.Read(reinterpret_cast<uint8_t *>(&ctx.planes), 2);
        source.Read(reinterpret_cast<uint8_t *>(&ctx.bits_per_pixel), 2);

        source.Read(reinterpret_cast<uint8_t *>(&ctx.compression_method), 4);

        source.Read(reinterpret_cast<uint8_t *>(&ctx.data_size), 4);

        source.Read(reinterpret_cast<uint8_t *>(&ctx.hppm), 4);
        source.Read(reinterpret_cast<uint8_t *>(&ctx.vppm), 4);

        source.Read(reinterpret_cast<uint8_t *>(&ctx.color_palettes), 4);
        source.Read(reinterpret_cast<uint8_t *>(&ctx.useful_colors), 4);
        return 0;
    }
    return ErrorCode::FileCorrupted;
}
int parse_color_table_header(ree::io::Source &source, BmpContext &ctx) {
    ctx.color_palette.resize(ctx.color_palettes);
    source.Read(reinterpret_cast<uint8_t *>(ctx.color_palette.data()),
        sizeof(Pixel) * ctx.color_palettes);
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
int parse_color_data(ree::io::Source &source, BmpContext &ctx) {
    uint32_t linesize = (ctx.bits_per_pixel * ctx.width + 31) / 32 * 4;

    std::vector<uint8_t> line;
    line.resize(linesize);

    for (uint32_t y = 0; y < ctx.height; ++y) {
        source.Read(line.data(), linesize);
        ree::io::LSigBitBuffer bitBuf(line.data(), line.size());
        for (uint32_t x = 0; x < ctx.width; ++x) {
            uint32_t v = bitBuf.ReadBits(7);

            Pixel p = ctx.color_palette[v];

            std::cout << v << ": " << (int)p.r << "," << (int)p.g << "," << (int)p.b << "," << (int)p.a << std::endl;

            uint32_t idx = (y * ctx.width + x) * 4;
            ctx.color[idx + 0] = p.r;
            ctx.color[idx + 1] = p.g;
            ctx.color[idx + 2] = p.b;
            ctx.color[idx + 3] = 0xff;
        }
    }
    return 0;
}

}
}
