#include "image.hpp"

namespace ree {
namespace image {
namespace process {

template Image<uint8_t> ImageFromIOImage<uint8_t>(const io::Image &srcImg);
template Image<uint16_t> ImageFromIOImage<uint16_t>(const io::Image &srcImg);

}
}
}