#include <ree/image/png.h>

#include <ree/unittest.h>

#include <iostream>

namespace ree {
namespace image {

R_TEST_F(Png, ParseHuffmanCode) {
    struct Leaf {
        int len;
        int code;
    };
    Leaf tree[8] = {
        {3, 0},
        {3, 0},
        {3, 0},
        {3, 0},
        {3, 0},
        {2, 0},
        {4, 0},
        {4, 0},
    };

    int maxLen = 5;
    int bl_count[5] = {0};
    int next_code[5] = {0};

    for (int i = 0; i < sizeof(tree) / sizeof(Leaf); ++i) {
        bl_count[tree[i].len]++;
    }

    int code = 0;
    bl_count[0] = 0;
    for (int bits = 1; bits <= maxLen; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }

    for (int i = 0; i < sizeof(tree) / sizeof(Leaf); ++i) {
        int len = tree[i].len;
        if (len != 0) {
            tree[i].code = next_code[len];
            next_code[len]++;
        }
    }

    for (int i = 0; i < sizeof(tree) / sizeof(Leaf); ++i) {
        std::cout << (char)('A' + i) << " " << tree[i].code << '\n';
    }
}

R_TEST_F(Png, ParsePng) {
    Png png;
    {
        auto source = ree::io::Source::SourceByPath("../test_assets/dot1.png");
        source->OpenToRead();
        auto ctx = png.CreateParseContext(source.get(), ParseOptions());
        Image img = png.ParseImage(ctx);
        source->Close();
    }
}

}
}
