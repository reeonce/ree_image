#include <ree/image/png_tests.h>
#include <ree/image/jpeg_tests.h>
#include <ree/image/ppm_tests.h>

int main(int argc, char const *argv[]) {
    // ree::image::testParsePng();

    // ree::image::testJpeg();

    ree::image::testParsePpm();
    ree::image::testComposePpm();
    return 0;
}