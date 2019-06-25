#pragma once

#include <iosfwd>

#include "src/formats/map.hpp"

namespace klmth {
  namespace map {
    Header parse_header(std::istream& in);
  }
}