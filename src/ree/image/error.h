#pragma once

#include <cstdint>
#include <vector>

namespace ree {
namespace image {

enum ErrorCode {
    OK = 0,
    IOFailed = -1,
    FileCorrupted = -2,
    NotMatch = -3,
};

}
}