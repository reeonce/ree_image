#include <ree/unittest.h>

#include <ree/image/ppm.h>
#include <ree/image/test_config.h>

namespace ree {
namespace image {

R_TEST_F(Ppm, ParsePpm) {
    Ppm ppm;
    Image img;
    {
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "dot1.ppm");
        source->OpenToRead();
        auto ctx = ppm.CreateParseContext(source.get(), ParseOptions());
        img = ppm.ParseImage(ctx);
        source->Close();
        R_ASSERT_EQ(ctx->errCode, ErrorCode::OK);
        R_ASSERT_EQ(img.width, 58);
        R_ASSERT_EQ(img.height, 50);
        R_ASSERT_EQ(img.depthBits, 8);
        R_ASSERT_EQ(img.colorspace, ColorSpace::RGB);
        R_ASSERT_EQ(img.data.size(), 58 * 50 * 3);

        std::vector<uint8_t> data;
        data.resize(img.width * img.height * 3 * 2);
        uint16_t *data16 = reinterpret_cast<uint16_t *>(data.data());
        for (int row = 0; row < img.height; ++row) {
            for (int col = 0; col < img.width; ++col) {
                int idx = row * img.width + col;
                data16[idx * 3] = static_cast<uint16_t>(img.data[idx * 3]) << 2;
                data16[idx * 3 + 1] = static_cast<uint16_t>(img.data[idx * 3 + 1]) << 2;
                data16[idx * 3 + 2] = static_cast<uint16_t>(img.data[idx * 3 + 2]) << 2;
            }
        }

        auto wsource = ree::io::Source::SourceByPath(kTestAssetsDir + "ret.ppm");
        wsource->OpenToWrite();
        Image img1(img.width, img.height, img.colorspace, std::move(data));
        img1.depthBits = 10;
        auto wctx = ppm.CreateComposeContext(wsource.get(), ComposeOptions());
        ppm.ComposeImage(wctx, img1);
        wsource->Close();
    }
    {
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "ret.ppm");
        source->OpenToRead();
        auto ctx = ppm.CreateParseContext(source.get(), ParseOptions());
        img = ppm.ParseImage(ctx);
        source->Close();
        R_ASSERT_EQ(ctx->errCode, ErrorCode::OK);
        R_ASSERT_EQ(img.width, 58);
        R_ASSERT_EQ(img.height, 50);
        R_ASSERT_EQ(img.depthBits, 10);
        R_ASSERT_EQ(img.colorspace, ColorSpace::RGB);
        R_ASSERT_EQ(img.data.size(), 58 * 50 * 3 * 2);
    }
}

}
}
