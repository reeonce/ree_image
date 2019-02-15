#include "jpeg.h"

#include <cmath>
#include <cassert>
#include <sstream>
#include <iostream>

#include <ree/io/bit_buffer.h>

#ifdef WIN32
#include <Winsock2.h>
#endif

namespace ree {
namespace image {

static std::vector<uint8_t> kMagicStr = {0xff, 0xd8, 0xff};

static ColorSpace kColorSpaces[] = {
    ColorSpace::Gray, // 0x00
    ColorSpace::Unknown, // 0x01
    ColorSpace::RGB, // 0x02
    ColorSpace::RGB, // 0x03
    ColorSpace::GrayAlpha, // 0x04
    ColorSpace::Unknown, // 0x05
    ColorSpace::RGBA, // 0x06
};
static int kComponents[] = { 1, 0, 3, 3, 2, 0, 4 };

struct Component {
    uint8_t id;
    uint8_t hSampleFactor;
    uint8_t vSampleFactor;
    uint8_t qhtCount;
};
    
struct HuffmanTable {
    struct Code {
        uint8_t len;
        uint8_t val;
    };
    std::vector<Code> codes;
    uint8_t type;
};

struct JpegParseContext : public ParseContext {
    using ParseContext::ParseContext;

    uint8_t precision;
    uint16_t width;
    uint16_t height;
    std::vector<Component> components;

    std::vector<HuffmanTable> hts;

    uint8_t depth;
    uint8_t colorType;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
    
    std::vector<uint8_t> color;
    size_t have = 0;
};

static void HandleMarker(uint8_t marker, JpegParseContext *ctx);
static uint8_t ReadEntropyCodedData(JpegParseContext *ctx);
static Image CreateImage(JpegParseContext *ctx);

std::vector<std::string> Jpeg::ValidExtensions() {
    return {"jpg", "JPG", "jpeg", "JPEG"};
}
std::vector<uint8_t> Jpeg::MagicNumber() {
    return kMagicStr;
}
std::string Jpeg::PreferredExtension() {
    return "jpg";
}

ParseContext *Jpeg::CreateParseContext(ree::io::Source *source,
    const ParseOptions &options) {
    return new JpegParseContext(source, options);
}
ComposeContext *Jpeg::CreateComposeContext(ree::io::Source *target,
    const ComposeOptions &options) {
    return new ComposeContext(target, options);
}

Image Jpeg::ParseImage(ParseContext *contex) {
    JpegParseContext *ctx = static_cast<JpegParseContext *>(contex);
    auto source = ctx->source;

    while (true) {
        uint8_t byte;
        int ret = source->Read(&byte, 1);
        if (ret != 0) {
            ctx->errCode = ErrorCode::IOFailed;
            break;
        }
        assert(byte == 0xff);

        source->Read(&byte, 1);
        HandleMarker(byte, ctx);
        if (ctx->done) {
            break;
        }
    }

    return CreateImage(ctx);
}

void Jpeg::ComposeImage(ComposeContext *ctx, const Image &image) {
    auto target = ctx->target;
}



Image CreateImage(JpegParseContext *ctx) {
    int components = kComponents[ctx->colorType];
    size_t bpp = (components * ctx->depth + 7) / 8;
    std::vector<uint8_t> data(ctx->width * ctx->height * components);

    size_t stride = (ctx->width * ctx->depth * components + 7) / 8 + 1;
    assert(stride * ctx->height == ctx->color.size());
    for (int row = 0; row < ctx->height; ++row) {
        auto bufferBeign = ctx->color.data() + row * stride;
        ree::io::BigEndianRLSBBuffer buffer(bufferBeign, stride);
        int filter = buffer.ReadBits(8);

        size_t dataBeginIndex = row * ctx->width * components;
        for (int i = 0; i < components; ++i) {
            data[dataBeginIndex + i] = buffer.ReadBits(ctx->depth);
        }
        for (int col = 1; col < ctx->width; ++col) {
            for (int i = 0; i < components; ++i) {
                uint32_t value = buffer.ReadBits(ctx->depth);
                switch (filter) {
                case 0:
                    break;
                case 1:
                    value += data[dataBeginIndex + col * 4 + i - bpp];
                    break;
                }
                data[dataBeginIndex + col * 4 + i] = value;
            }
        }
    }
    return Image(ctx->width, ctx->height, kColorSpaces[ctx->colorType], 
        std::move(data));
}

void HandleMarker(uint8_t marker, JpegParseContext *ctx) {
    auto source = ctx->source;

    uint16_t len = 0;
    std::vector<uint8_t> payload;
    if (marker > 0xda || marker < 0xd0) {
        source->Read(reinterpret_cast<uint8_t *>(&len), 2);
        len = ntohs(len);

        payload.resize(len - 2);
        source->Read(payload.data(), len - 2);
    }
    printf("marker %x, len: %x\n", marker, len);
    if (marker == 0xd8) { // SOI 
        return;
    }
    if (marker == 0xd9) { // EOI 
        ctx->done = true;
        return;
    }
    if (marker >= 0xe0 && marker <= 0xef) { // APPn
        return;
    }
    if (marker == 0xc0) { // SOF0
        ree::io::BigEndianRLSBBuffer bitBuffer(payload.data(), payload.size());
        ctx->precision = bitBuffer.ReadBits(8);
        ctx->height = bitBuffer.ReadBits(16);
        ctx->width = bitBuffer.ReadBits(16);
        uint8_t comp = bitBuffer.ReadBits(8);
        ctx->components.resize(comp);
        for (int i = 0; i < comp; ++i) {
            ctx->components[i].id = bitBuffer.ReadBits(8);
            ctx->components[i].hSampleFactor = bitBuffer.ReadBits(4);
            ctx->components[i].vSampleFactor = bitBuffer.ReadBits(4);
            ctx->components[i].qhtCount = bitBuffer.ReadBits(8);
        }
        return;
    }
    if (marker == 0xc2) { // SOF2
        return;
    }
    if (marker == 0xc4) { // DHT
        size_t cursor = 0;
        uint8_t hti = payload[cursor++];
        uint8_t noht = hti >> 5;
        uint8_t htizero = noht & 0x07;
        assert(htizero == 0);
        
        HuffmanTable ht;
        ht.codes.resize(256);
        ht.type = (noht >> 4) & 0x01;

        uint8_t nLens[16];
        size_t totalCodes = 0;
        for (int i = 0; i < 16; ++i) {
            nLens[i] = payload[cursor++];
            totalCodes += nLens[i];
        }
        std::vector<uint8_t> orders(totalCodes);
        for (int i = 0; i < totalCodes; ++i) {
            orders[i] = payload[cursor++];
        }
        
        return;
    }
    if (marker == 0xdd) { // DRI
        return;
    }
    if (marker == 0xdb) { // DQT
        return;
    }
    if (marker == 0xda) { // SOS
        marker = ReadEntropyCodedData(ctx);
        HandleMarker(marker, ctx);
        return;
    }
}
uint8_t ReadEntropyCodedData(JpegParseContext *ctx) {
    auto source = ctx->source;
    uint8_t prevByte = 0x00;
    while (true) {
        uint8_t byte;
        source->Read(&byte, 1);
        if (prevByte == 0xff) {
            if (byte <= 0xd7 && byte >= 0xd0) {
                //
            } else if (byte != 0x00) {
                return byte;
            }
        }
        prevByte = byte;
    }
}

}
}
