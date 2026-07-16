// exceptions.cc
#include "exceptions.h"

namespace lightknight::exceptions {

FENException::FENException(const std::string& msg)
    : std::runtime_error(msg) {};

} // namespace lightknight::exceptions
