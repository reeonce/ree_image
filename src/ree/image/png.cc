#include "png.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>
#include <sstream>
#include <iostream>

#include <zlib.h>
#include <ree/io/bit_buffer.h>

#define WITH_LIBZ 1

#ifdef WIN32
#include <Winsock2.h>
#endif

namespace ree {
namespace image {

static std::vector<uint8_t> kMagicStr = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

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

struct Chunk {
    Chunk(uint32_t length, uint32_t type, const std::vector<uint8_t> &&payload,
        uint32_t crcValue)
        : length(length),
          type(type),
          payload(std::move(payload)),
          crc(crcValue) {
    }

    bool Empty() const { return type == 0; }

    uint32_t length = 0;
    uint32_t type;
    std::vector<uint8_t> payload;
    uint32_t crc;
};

struct PngParseContext : public ParseContext {
    using ParseContext::ParseContext;

    int width;
    int height;
    uint8_t depth;
    uint8_t colorType;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
    
#if WITH_LIBZ
    z_stream strm;
#endif
    std::vector<uint8_t> color;
    size_t have = 0;
};
    
static bool ParseChunk(const Chunk &chunk, PngParseContext *ctx);
static Chunk ReadChunk(PngParseContext *ctx);
static Image CreateImage(PngParseContext *ctx);

static void LibZInflate(const uint8_t *data, size_t size, PngParseContext *ctx);
static void DecFixedHuffmanDeflate(PngParseContext *ctx);
static void DecDynamicHuffmanDeflate(PngParseContext *ctx,
    ree::io::BigEndianRLSBBuffer &bitBuffer);

std::vector<std::string> Png::ValidExtensions() {
    return {"png", "PNG", };
}
std::vector<uint8_t> Png::MagicNumber() {
    return kMagicStr;
}
std::string Png::PreferredExtension() {
    return "png";
}

ParseContext *Png::CreateParseContext(ree::io::Source *source,
    const ParseOptions &options) {
    return new PngParseContext(source, options);
}
ComposeContext *Png::CreateComposeContext(ree::io::Source *target,
    const ComposeOptions &options) {
    return new ComposeContext(target, options);
}

Image Png::ParseImage(ParseContext *contex) {
    PngParseContext *ctx = static_cast<PngParseContext *>(contex);
    auto source = ctx->source;

    std::vector<uint8_t> magicStr(kMagicStr.size());
    source->Read(magicStr.data(), magicStr.size());
    if (magicStr != kMagicStr) {
        ctx->errCode = ErrorCode::NotMatch;
        return Image();
    }

    while (true) {
        Chunk chunk = ReadChunk(ctx);
        if (chunk.Empty()) {
            break;
        }
        if (!ParseChunk(chunk, ctx)) {
            break;
        }
        if (ctx->done) {
            break;
        }
    }

    if (ctx->errCode != ErrorCode::OK) {
        return Image();
    }

    return CreateImage(ctx);
}

void Png::ComposeImage(ComposeContext *ctx, const Image &image) {
    auto target = ctx->target;
}


Chunk ReadChunk(PngParseContext *ctx) {
    auto source = ctx->source;

    uint32_t length = 0;
    source->Read(reinterpret_cast<uint8_t *>(&length), 4);
    length = ntohl(length);

    uint32_t type;
    source->Read(reinterpret_cast<uint8_t *>(&type), 4);
    type = ntohl(type);

    std::vector<uint8_t> payload;
    if (length > 0) {
        payload.resize(length);
        source->Read(payload.data(), length);
    }

    uint32_t crc;
    source->Read(reinterpret_cast<uint8_t *>(&crc), 4);
    crc = ntohl(crc);

    return Chunk(length, type, std::move(payload), crc);
}

bool ParseChunk(const Chunk &chunk, PngParseContext *ctx) {
    uint32_t type = chunk.type;
    if (type == 'IHDR') {
        const uint8_t *cursor = chunk.payload.data();
        
        std::copy(cursor, cursor + 4, reinterpret_cast<uint8_t *>(&ctx->width));
        cursor += 4;
        ctx->width = ntohl(ctx->width);
        
        std::copy(cursor, cursor + 4, reinterpret_cast<uint8_t *>(&ctx->height));
        cursor += 4;
        ctx->height = ntohl(ctx->height);
        
        std::copy(cursor, cursor + 1, &ctx->depth);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx->colorType);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx->compression);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx->filter);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx->interlace);
        cursor += 1;

        ctx->strm.zalloc = Z_NULL;
        ctx->strm.zfree = Z_NULL;
        ctx->strm.opaque = Z_NULL;
        ctx->strm.avail_in = 0;
        ctx->strm.next_in = Z_NULL;
        int ret = inflateInit(&ctx->strm);
        assert(ret == 0);
    } else if (type == 'iCCP') {
        
    } else if (type == 'pHYs') {
        
    } else if (type == 'iTXt') {
        std::string xml(chunk.payload.begin(), chunk.payload.end());
        std::cout << xml << std::endl;
    } else if (type == 'iDOT') {
        
    } else if (type == 'PLTE') {
        
    } else if (type == 'IDAT') {
#if WITH_LIBZ
        LibZInflate(chunk.payload.data(), chunk.payload.size(), ctx);
#else
        ree::io::BigEndianRLSBBuffer bitBuffer(chunk.payload.data(),
            chunk.payload.size());
        uint8_t cmf = bitBuffer.NextBits(8);
        uint8_t cm = bitBuffer.ReadBits(4);
        assert(cm == 0x08);
        
        uint8_t cinfo = bitBuffer.ReadBits(4);
        int windowBase = cinfo + 8;
        int windowSize = 1 << windowBase;
        
        uint8_t flg = bitBuffer.NextBits(8);
        assert((cmf * 256 + flg) % 31 == 0);
        
        bitBuffer.ReadBits(5);
        uint8_t fdict = bitBuffer.ReadBits(1);
        if (fdict) {
            // TODO: FDICT
        }
        uint8_t flevel = bitBuffer.ReadBits(2);
        
        auto dataIt = chunk.payload.begin() + 2;
        uint8_t bfinal = 0;
        do {
            bfinal = bitBuffer.ReadBits(1);
            uint8_t btype = bitBuffer.ReadBits(2);
            assert(btype != 0x11);
            
            switch (btype) {
            case 0x00: {
                bitBuffer.ReadBits(5);
                
                uint32_t len = bitBuffer.ReadBits(16);
                uint32_t nlen = bitBuffer.ReadBits(16);
                
                std::vector<uint8_t> colordata(dataIt + 1, dataIt + 1 + len);
            }
                break;
            case 0x01:
                DecFixedHuffmanDeflate(ctx);
                break;
            case 0x02:
                DecDynamicHuffmanDeflate(ctx, bitBuffer);
                break;
            }
        } while (bfinal == 0);
#endif
    } else if (type == 'IEND') {
        assert(chunk.length == 0);
        ctx->done = true;
    }
    return true;
}
    
void LibZInflate(const uint8_t *data, size_t size, PngParseContext *ctx) {
    ctx->strm.next_in = const_cast<uint8_t *>(data);
    ctx->strm.avail_in = size;
    
    uint8_t chunk[32768];
    
    ctx->strm.next_out = chunk;
    ctx->strm.avail_out = sizeof(chunk);
    int ret = inflate(&ctx->strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
    switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     /* and fall through */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            assert(false);
    }
    
    size_t outSize = sizeof(chunk) - ctx->strm.avail_out;
    ctx->color.insert(ctx->color.end(), chunk, chunk + outSize); 
    
    if (ret == Z_STREAM_END) {
        inflateEnd(&ctx->strm);
    }
}

void DecFixedHuffmanDeflate(PngParseContext *ctx) {
    
}
    
void DecDynamicHuffmanDeflate(PngParseContext *ctx,
    ree::io::BigEndianRLSBBuffer &bitBuffer) {
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    
    int nLen = bitBuffer.ReadBits(5) + 257;
    int nDis = bitBuffer.ReadBits(5) + 1;
    int ncLen = bitBuffer.ReadBits(4) + 4;
    
    std::vector<uint8_t> cLens(ncLen);
    for (int i = 0; i < ncLen; i++) {
        cLens[order[i]] = bitBuffer.ReadBits(3);
    }
    
    uint32_t len = bitBuffer.ReadBits(16);
    uint32_t nlen = bitBuffer.ReadBits(16);
}

Image CreateImage(PngParseContext *ctx) {
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

}
}
