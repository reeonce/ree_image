#include "file_format_interface.h"

namespace ree {
namespace image {

ParseContext::ParseContext(ree::io::Source *src, const ParseOptions &opt)
    : source(src),
      options(opt),
      errCode(ErrorCode::OK),
      done(false) {
}

ComposeContext::ComposeContext(ree::io::Source *tgt, const ParseOptions &opt)
    : target(tgt),
      options(opt),
      errCode(ErrorCode::OK) {
}

FileFormatInterface::~FileFormatInterface() {}

}
}