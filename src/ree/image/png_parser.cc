#include "png_parser.h"

#include <fstream>
#include <iostream>

namespace ree {
namespace image {

static std::vector<uint8_t> kMagicStr = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

struct Chunk {
    uint32_t length;
    char type[4];
    std::vector<uint8_t> data;
    uint32_t crc;
};

struct Context {
    int width;
    int height;
    uint8_t depth;
    uint8_t colorType;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;

    int err = 0;
    bool done = false;
};
    
    
static bool ParseChunk(const Chunk &chunk, Context &ctx) {
    uint32_t type;
    std::copy(chunk.type, chunk.type + 4, reinterpret_cast<char *>(&type));
    type = ntohl(type);
    if (type == 'IHDR') {
        const uint8_t *cursor = chunk.data.data();
        
        std::copy(cursor, cursor + 4, reinterpret_cast<uint8_t *>(&ctx.width));
        cursor += 4;
        ctx.width = ntohl(ctx.width);
        
        std::copy(cursor, cursor + 4, reinterpret_cast<uint8_t *>(&ctx.height));
        cursor += 4;
        ctx.height = ntohl(ctx.height);
        
        std::copy(cursor, cursor + 1, &ctx.depth);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx.colorType);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx.compression);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx.filter);
        cursor += 1;
        
        std::copy(cursor, cursor + 1, &ctx.interlace);
        cursor += 1;
        
        assert(cursor == chunk.data.end().operator->());
    } else if (type == 'iCCP') {
        
    } else if (type == 'pHYs') {
        
    } else if (type == 'iTXt') {
        std::string xml(chunk.data.begin(), chunk.data.end());
        std::cout << xml << std::endl;
    } else if (type == 'iDOT') {
        
    } else if (type == 'PLTE') {
        
    } else if (type == 'IDAT') {
        const uint8_t *cursor = chunk.data.data();
        uint8_t cmf = *(cursor++);
        int cm = cmf & 0x0f;
        assert(cm == 0x08);
        
        int windowBase = (cmf >> 4) + 8;
        int windowSize = 1 << windowBase;
        
        uint8_t flg = *(cursor++);
        
        assert((cmf * 256 + flg) % 31 == 0);
        
        if (flg & 0x20) {
            // TODO: FDICT
        }
        uint8_t flevel = flg >> 6;
        
        uint8_t bfinal = 0;
        do {
            const uint8_t *block = cursor;
            bfinal = *block >> 7;
            uint8_t btype = (*block >> 5) & 0x03;
            assert(btype != 0x11);
        } while (bfinal == 0);
        
    } else if (type == 'IEND') {
        ctx.done = true;
        assert(chunk.length == 0);
    }
    return true;
}
    
static bool ReadChunk(std::ifstream &f, Chunk &chunk, Context &ctx) {
    if (!f.read(reinterpret_cast<char *>(&chunk.length), 4)) {
        ctx.err = -1;
        return false;
    }
    chunk.length = ntohl(chunk.length);
    if (!f.read(chunk.type, 4)) {
        ctx.err = -1;
        return false;
    }
    if (chunk.length > 0) {
        chunk.data.resize(chunk.length);
        if (!f.read(reinterpret_cast<char *>(chunk.data.data()), chunk.length)) {
            ctx.err = -1;
            return false;
        }
    }
    if (!f.read(reinterpret_cast<char *>(&chunk.crc), 4)) {
        ctx.err = -1;
        return false;
    }
    chunk.crc = ntohl(chunk.crc);
    return true;
}

int PngParser::ParseFile(const std::string &path, Image &image) {
    Context ctx;

    std::ifstream f(path, std::ifstream::binary);
    if (!f.is_open()) {
        return -1;
    }

    std::vector<uint8_t> magicStr;
    magicStr.resize(8);

    if (!f.read(reinterpret_cast<char *>(magicStr.data()), magicStr.size())) {
        return -1;
    }

    if (magicStr != kMagicStr) {
        return -2;
    }

    while (true) {
        Chunk chunk;
        if (!ReadChunk(f, chunk, ctx)) {
            break;
        }
        if (!ParseChunk(chunk, ctx)) {
            break;
        }
        if (ctx.done) {
            break;
        }
    }

    if (ctx.err != 0) {
        return ctx.err;
    }

    image.width = ctx.width;
    image.height = ctx.height;
    if (ctx.colorType == 0x06) {
        image.colorspace = ColorSpace::RGBA;
    } else if (ctx.colorType == 0x00) {
        image.colorspace = ColorSpace::Gray;
    } else if (ctx.colorType == 0x02) {
        image.colorspace = ColorSpace::RGB;
    } else if (ctx.colorType == 0x03) {
        // TODO: palette
        image.colorspace = ColorSpace::RGB;
    } else if (ctx.colorType == 0x04) {
        image.colorspace = ColorSpace::GrayAlpha;
    }

    return true;
}


}
}
