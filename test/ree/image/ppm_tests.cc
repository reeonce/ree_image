#include <ree/unittest.h>

#include <ree/image/ppm.h>

namespace ree {
namespace image {

R_TEST_F(Ppm, ParsePpm) {
    Ppm ppm;
    Image img;
    {
        int ret = ppm.Parse("../test_assets/dot1.ppm", img);
        R_ASSERT_EQ(ret, ErrorCode::OK);
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
        Image img1(img.width, img.height, img.colorspace, std::move(data));
        img1.depthBits = 10;
        ppm.Compose(img1, "../test_assets/ret.ppm");
    }
    {
        int ret = ppm.Parse("../test_assets/ret.ppm", img);
        R_ASSERT_EQ(ret, ErrorCode::OK);
        R_ASSERT_EQ(img.width, 58);
        R_ASSERT_EQ(img.height, 50);
        R_ASSERT_EQ(img.depthBits, 10);
        R_ASSERT_EQ(img.colorspace, ColorSpace::RGB);
        R_ASSERT_EQ(img.data.size(), 58 * 50 * 3 * 2);
    }
}

}
}
