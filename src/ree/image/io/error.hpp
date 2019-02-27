#pragma once

#include <cstdint>
#include <vector>
#include <exception>
#include <string>

namespace ree {
namespace image {
namespace io {

class FileCorruptedException : public std::exception {
public:
    explicit FileCorruptedException (const std::string& what_arg)
        : what_(std::string("File corrupted with " + what_arg)) {
    }
    const char* what() const noexcept { return what_.c_str(); }

private:
    std::string what_;
};

class NotImplementException : public std::exception {
    const char* what() const noexcept {
        return "Not implemented!\n";
    }
};

class UnknownFormatException : public std::exception {
    const char* what() const noexcept {
        return "Unknown file format!\n";
    }
};

}

}
}