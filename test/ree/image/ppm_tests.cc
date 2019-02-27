#include <ree/unittest.h>

#include <ree/image/io/ppm.hpp>
#include <ree/image/test_config.h>

namespace ree {
namespace image {
namespace io {

R_TEST_F(Ppm, ParsePpm) {
    Ppm ppm;
    {
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "dot1.ppm");
        source->OpenToRead();
        auto ctx = ppm.CreateParseContext(source.get(), LoadOptions());
		Image img = ppm.LoadImage(ctx);
        source->Close();
        R_ASSERT_EQ(img.Width(), 58);
        R_ASSERT_EQ(img.Height(), 50);
        R_ASSERT_EQ(img.DepthBits(), 8);
        R_ASSERT_EQ(img.ColorSpace(), ColorSpace::RGB);
        R_ASSERT_EQ(img.Data().size(), 58 * 50 * 3);

        std::vector<uint8_t> data;
        data.resize(img.Width() * img.Height() * 3 * 2);
        uint16_t *data16 = reinterpret_cast<uint16_t *>(data.data());
        for (int row = 0; row < img.Height(); ++row) {
            for (int col = 0; col < img.Width(); ++col) {
                int idx = row * img.Width() + col;
                data16[idx * 3] = static_cast<uint16_t>(img.Data()[idx * 3]) << 2;
                data16[idx * 3 + 1] = static_cast<uint16_t>(img.Data()[idx * 3 + 1]) << 2;
                data16[idx * 3 + 2] = static_cast<uint16_t>(img.Data()[idx * 3 + 2]) << 2;
            }
        }

        auto wsource = ree::io::Source::SourceByPath(kTestAssetsDir + "ret.ppm");
        wsource->OpenToWrite();
        Image img1(img.Width(), img.Height(), img.ColorSpace(), 10, std::move(data));
        auto wctx = ppm.CreateComposeContext(wsource.get(), WriteOptions());
        ppm.WriteImage(wctx, img1);
        wsource->Close();
    }
    {
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "ret.ppm");
        source->OpenToRead();
        auto ctx = ppm.CreateParseContext(source.get(), LoadOptions());
		Image img = ppm.LoadImage(ctx);
        source->Close();
        R_ASSERT_EQ(img.Width(), 58);
        R_ASSERT_EQ(img.Height(), 50);
        R_ASSERT_EQ(img.DepthBits(), 10);
        R_ASSERT_EQ(img.ColorSpace(), ColorSpace::RGB);
        R_ASSERT_EQ(img.Data().size(), 58 * 50 * 3 * 2);
    }
}

}
}
}
