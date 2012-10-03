#include <stdexcept>
#pragma once
class FormatException: public std::runtime_error {
public:
    explicit FormatException(std::string msg) : std::runtime_error(msg) { }
};

