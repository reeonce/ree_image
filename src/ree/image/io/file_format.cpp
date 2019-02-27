#include "file_format.hpp"

namespace ree {
namespace image {
namespace io {

LoadContext::LoadContext(ree::io::Source *src, const LoadOptions &opt)
    : source(src),
      options(opt),
      done(false) {
}

WriteContext::WriteContext(ree::io::Source *tgt, const LoadOptions &opt)
    : target(tgt),
      options(opt) {
}

}
}
}