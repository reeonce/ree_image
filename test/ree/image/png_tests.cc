#include "png_tests.h"

#include <ree/image/png_parser.h>

namespace ree {
namespace image {

void testParsePng() {
    PngParser parser;
    Image img;
    parser.ParseFile("../test_assets/dot1.png", img);
}

}
}