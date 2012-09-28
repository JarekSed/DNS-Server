#include <stdexcept>

class FormatException: public std::runtime_error {
public:
  explicit FormatException(string msg) : std::runtime_error(msg) { }
};

