#include <ree/image/io/png.hpp>
#include <ree/image/io/ppm.hpp>


#include <iostream>

#include <ree/unittest.h>
#include <ree/image/test_config.h>
#include <ree/image/process/image.hpp>

namespace ree {
namespace image {
namespace io {

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
    int next_code[8] = {0};

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
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "dot1.png");
        source->OpenToRead();
        auto ctx = png.CreateParseContext(source.get(), LoadOptions());
        Image img = png.LoadImage(ctx);
        source->Close();

		process::Image<uint8_t> pImg = process::ImageFromIOImage<uint8_t>(img);
		process::Image<uint8_t> pCtvedImg = pImg.ConvertToColor(ColorSpace::RGB);
		Image wImg = pCtvedImg.ToIOImage();

        Ppm ppm;
        auto wsource = ree::io::Source::SourceByPath(kTestAssetsDir + "png_ret.ppm");
        wsource->OpenToWrite();
        auto wctx = ppm.CreateComposeContext(wsource.get(), WriteOptions());
        ppm.WriteImage(wctx, wImg);
        wsource->Close();
    }
}

}
}
}
