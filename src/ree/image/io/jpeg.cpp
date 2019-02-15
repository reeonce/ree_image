#include "jpeg.hpp"

#include <cmath>
#include <cassert>
#include <sstream>
#include <iostream>
#include <array>
#include <algorithm>
#include <queue>
#include <bitset>

#include <ree/io/bit_buffer.h>
#include <ree/image/io/error.hpp>

#ifdef WIN32
#include <Winsock2.h>
#undef  LoadImage
#endif

namespace ree {
namespace image {
namespace io {

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
static std::array<int, 7> kComponents = { 1, 0, 3, 3, 2, 0, 4 };

static constexpr size_t nLenSize = 16;
    
struct Component {
    uint8_t id;
    uint8_t hSampleFactor;
    uint8_t vSampleFactor;
    uint8_t qhtCount;

    uint8_t dcHtId;
    uint8_t acHtId;
};
    
struct HuffmanTable {
    struct Node {
        uint8_t symbol;
        bool isLeaf = false;
        
        std::unique_ptr<Node> lChild; // 0
        std::unique_ptr<Node> rChild; // 1
    };
    
    std::unique_ptr<Node> root;
    
    void PrintLevelOrder() const {
        std::queue<Node *> queue;
        queue.push(root.get());
        size_t level = 0;
        size_t levelSize = 1;
        size_t levelIdx = 0;
        
        while (queue.size() > 0) {
            if (levelIdx >= levelSize) {
                ++level;
                levelSize = queue.size();
                levelIdx = 0;
                std::cout << "\n" << level << ": ";
            }

            auto node = queue.front();
            queue.pop();
            levelIdx++;
            if (node->isLeaf) {
                std::cout << std::to_string(node->symbol) << " ";
            }

            if (node->lChild) {
                queue.push(node->lChild.get());
            }
            if (node->rChild) {
                queue.push(node->rChild.get());
            }
        }
    }
};

using QuantizationTable = std::array<uint16_t, 64>;

struct JpegParseContext : public LoadContext {
    using LoadContext::LoadContext;

    uint8_t precision;
    uint16_t width;
    uint16_t height;
    std::vector<Component> components;

    std::vector<HuffmanTable> dcHt;
    std::vector<HuffmanTable> acHt;

    std::vector<QuantizationTable> qts;

    uint8_t depth;
    uint8_t colorType;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
    
    std::vector<uint8_t> color;
    size_t have = 0;
};

static void HandleMarker(uint8_t marker, JpegParseContext *ctx);
static void SetupHuffmanTree(HuffmanTable *ht,
    const std::vector<std::vector<uint8_t>> &lenSymbols);
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

LoadContext *Jpeg::CreateParseContext(ree::io::Source *source,
    const LoadOptions &options) {
    return new JpegParseContext(source, options);
}
WriteContext *Jpeg::CreateComposeContext(ree::io::Source *target,
    const WriteOptions &options) {
    return new WriteContext(target, options);
}

Image Jpeg::LoadImage(LoadContext *contex) {
    JpegParseContext *ctx = static_cast<JpegParseContext *>(contex);
    auto source = ctx->source;

    while (true) {
        uint8_t byte;
        source->Read(&byte, 1);
        assert(byte == 0xff);

        source->Read(&byte, 1);
        HandleMarker(byte, ctx);
        if (ctx->done) {
            break;
        }
    }

    return CreateImage(ctx);
}

void Jpeg::WriteImage(WriteContext *ctx, const Image &image) {
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
        ctx->depth, std::move(data));
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
        uint8_t htNo = hti & 0x0f;
        assert(htNo <= 3);
        uint8_t htizero = hti >> 5;
        assert(htizero == 0);
        
        uint8_t type = (hti >> 4) & 0x01;
        HuffmanTable *ht;
        if (type == 0) {
            ctx->dcHt.push_back(HuffmanTable());
            ht = &ctx->dcHt.back();
        } else {
            ctx->acHt.push_back(HuffmanTable());
            ht = &ctx->acHt.back();
        }

        std::array<uint8_t, nLenSize> nLens;
        size_t totalCodes = 0;
        uint8_t maxDepth = 0;
        for (uint8_t i = 0; i < nLenSize; ++i) {
            nLens[i] = payload[cursor++];
            totalCodes += nLens[i];
            if (nLens[i] > 0) { maxDepth = i; }
        }
        
        std::vector<std::vector<uint8_t>> lenSymbols(maxDepth + 1);
        for (uint8_t i = 0; i < lenSymbols.size(); ++i) {
            lenSymbols[i].resize(nLens[i]);
            for (uint8_t j = 0; j < nLens[i]; j++) {
                lenSymbols[i][j] = payload[cursor++];
            }
        }
        
        SetupHuffmanTree(ht, lenSymbols);
        
        return;
    }
    if (marker == 0xdd) { // DRI
        return;
    }
    if (marker == 0xdb) { // DQT
        size_t cursor = 0;
        uint8_t qti = payload[cursor++];
        uint8_t qtNo = qti & 0x0f;
        assert(qtNo <= 3);
        uint8_t qt_precision = qti >> 4;
        
        ctx->qts.push_back(QuantizationTable());

        QuantizationTable &qt = ctx->qts.back();
        if (qt_precision == 0) {
            for (size_t i = 0; i < qt.size(); ++i) {
                qt[i] = payload[cursor++];
            }
        } else {
            for (size_t i = 0; i < qt.size(); ++i) {
                qt[i] = payload[cursor++];
                qt[i] = qt[i] << 8 | payload[cursor++];
            }
        }
        return;
    }
    if (marker == 0xda) { // SOS
        marker = ReadEntropyCodedData(ctx);
        HandleMarker(marker, ctx);
        return;
    }
}
    
void SetupHuffmanTree(HuffmanTable *ht,
    const std::vector<std::vector<uint8_t>> &lenSymbols) {
    ht->root.reset(new HuffmanTable::Node());
    
    std::vector<HuffmanTable::Node *> nonLeafNodes = { ht->root.get() };
    for (size_t i = 0; i < lenSymbols.size(); ++i) {
        std::vector<HuffmanTable::Node *> childNodes;
        childNodes.reserve(2 * nonLeafNodes.size() - lenSymbols[i].size());
        
        size_t symbolsIndex = 0;
        for (auto node : nonLeafNodes) {
            node->lChild.reset(new HuffmanTable::Node());
            node->rChild.reset(new HuffmanTable::Node());

            std::array<HuffmanTable::Node *, 2> children = {
                node->lChild.get(), node->rChild.get()
            };
            for (auto child : children) {
                if (symbolsIndex < lenSymbols[i].size()) {
                    child->symbol = lenSymbols[i][symbolsIndex];
                    child->isLeaf = true;
                    ++symbolsIndex;
                } else {
                    childNodes.push_back(child);
                }
            }
        }
        nonLeafNodes = std::move(childNodes);
    }
//    ht->PrintLevelOrder();
}

uint8_t ReadEntropyCodedData(JpegParseContext *ctx) {
    auto source = ctx->source;

    uint16_t len = 0;
    std::vector<uint8_t> payload;
    source->Read(reinterpret_cast<uint8_t *>(&len), 2);
    len = ntohs(len);
    
    uint8_t components;
    source->Read(&components, 1);

    assert(len == 6 + 2 * components);
    for (uint8_t i = 0; i < components; ++i) {
        uint8_t cid;
        source->Read(&cid, 1);

        uint8_t tableInfo;
        source->Read(&tableInfo, 1);

        auto find = std::find_if(ctx->components.begin(), ctx->components.end(),
            [cid](const Component &component) { return component.id == cid; });
        if (find == ctx->components.end()) {
            throw FileCorruptedException("component not found");
        }
        find->acHtId = tableInfo >> 4;
        find->dcHtId = tableInfo & 0x0f;
    }
    
    std::array<uint8_t, 3> skip;
    source->Read(skip.data(), skip.size());

    auto componentIt = ctx->components.begin();
    HuffmanTable &dcht = ctx->dcHt[componentIt->dcHtId];
    HuffmanTable &acht = ctx->acHt[componentIt->acHtId];
    
    HuffmanTable::Node *dcNode = dcht.root.get();
    HuffmanTable::Node *acNode = acht.root.get();
    
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

        std::bitset<8> bits(byte);
        for (size_t i = 0; i < bits.size(); ++i) {
            dcNode = bits[i] ? dcNode->rChild.get() : dcNode->lChild.get();
            if (dcNode->isLeaf) {
                // TODO: append data
                dcNode = dcht.root.get();
            }
        }
    }
}

}
}
}
