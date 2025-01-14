#include "src/formats/lst_reader.hpp"

#include <array>
#include <istream>

std::vector<std::string> klmth::lst::parse_file(std::istream& in) {
  
  enum class State {
    start,
    content,
    ws,
    comment,
  };
  
  std::array<char, 1<<16> buf;
  std::string ws;
  std::string cur;
  State st{State::start};
  std::vector<std::string> ret;

  // Parsing as a streaming state machine. Reads are done into a 2^16
  // byte buffer to reduce IO overhead.
  //
  // LST files can contain line comments (start with ';') and
  // leading/trailing whitespace around each entry. This parser tries
  // to skip all of that so that the output contains clean, trimmed
  // entries.
  while (!in.eof()) {
    in.read(buf.data(), buf.size());
    auto avail = in.gcount();
    
    for (int i = 0; i < avail; ++i) {
      char c = buf[i];

      switch (c) {
      case '\r':
        break;
      case '\n':
        ret.emplace_back(std::move(cur));
        cur.clear();
        ws.clear();
        st = State::start;
        break;
      case ';':
        ws.clear();
        st = State::comment;
        break;
      case ' ':
      case '\t':
        switch (st) {
        case State::start:
          break;
        case State::content:
          ws += c;
          st = State::ws;
          break;
        case State::ws:
          ws += c;
          break;
        case State::comment:
          break;
        }
        break;
      default:  // normal char
        switch (st) {  
        case State::start:
          cur += c;
          st = State::content;
          break;
        case State::content:
          cur += c;
          break;
        case State::ws:
          cur += ws;
          cur += c;
          ws.clear();
          st = State::content;
          break;
        case State::comment:
          break;
        }
        break;
      }
    }
  }

  // trailing newline is not specified
  if (!cur.empty()) {
    ret.emplace_back(std::move(cur));
  }

  return ret;
}
