#include "png_tests.h"

#include <ree/image/ppm.h>

namespace ree {
namespace image {


void testParsePpm() {
    Ppm ppm;
    Image img;
    ppm.Parse("../test_assets/dot1.ppm", img);
}
void testComposePpm() {
    Ppm ppm;
    Image img;
    img.width = 16;
    img.height = 16;
    img.data.resize(img.width * img.height * 3);
    for (int row = 0; row < img.height; ++row) {
        for (int col = 0; col < img.width; ++col) {
            int idx = row * img.width + col;
            img.data[idx * 3] = idx;
            img.data[idx * 3 + 1] = 255 - idx;
            img.data[idx * 3 + 2] = idx;
        }
    }
    ppm.Compose(img, "../test_assets/ret.ppm");
}

}
}