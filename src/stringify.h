#include <iomanip>
#include <sstream>
#include <string>

static std::string escape_json_string(const std::string &s) {
  std::ostringstream o;
  for (char c : s) {
    switch (c) {
    case '"':
      o << "\\\"";
      break;
    case '\\':
      o << "\\\\";
      break;
    case '\b':
      o << "\\b";
      break;
    case '\f':
      o << "\\f";
      break;
    case '\n':
      o << "\\n";
      break;
    case '\r':
      o << "\\r";
      break;
    case '\t':
      o << "\\t";
      break;
    default:
      if ('\x00' <= c && c <= '\x1f') {
        o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
          << static_cast<uint32_t>(c);
      } else {
        o << c;
      }
    }
  }
  return o.str();
}