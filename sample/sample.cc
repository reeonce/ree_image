#include <iostream>
#include <fstream>
#include <string>
#include <exception>

#include <ree/image/io/image.hpp>
#include <ree/image/io/error.hpp>

int main(int argc, char const *argv[]) {
    auto source = ree::io::Source::SourceByPath("./test/lena.jpg");
    ree::image::io::Image img;
    try {
        img = ree::image::io::Image::Load(source.get());
    } catch (std::ios_base::failure ioe) {
        std::cout << "load failed as io failure." << std::endl;
    } catch (ree::image::io::FileCorruptedException fce) {
        std::cout << "load failed as file corrupted." << std::endl;
    }

    auto target = ree::io::Source::SourceByPath("./test/lena_xx.ppm");
    try {
        img.WriteTo(target.get());
    } catch (std::ios_base::failure ioe) {
        std::cout << "write failed as io failure." << std::endl;
    }
    return 0;
}