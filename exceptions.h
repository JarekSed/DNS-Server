#include <stdexcept>

class DnsFormatException: public std::runtime_error {
public:
  explicit DnsFormatException(string msg) : std::runtime_error(msg) { }
};

