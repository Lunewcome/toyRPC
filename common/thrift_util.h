#include <boost/shared_ptr.hpp>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TDebugProtocol.h>

#include "common/basics.h"
#include "common/log.h"
#include "common/string_util.h"

using namespace ::apache::thrift;  // NOLINT
using namespace ::apache::thrift::protocol;  // NOLINT
using namespace ::apache::thrift::transport;  // NOLINT

// This method is unsafe and could be used only when object is clean.
template <typename T>
inline bool FromStringToThriftFast(const std::string &buffer, T *object) {
  boost::shared_ptr<TMemoryBuffer> membuffer(new TMemoryBuffer(
        const_cast<uint8*>(reinterpret_cast<const uint8*>(buffer.c_str())),
        buffer.size()));
  boost::shared_ptr<TProtocol> binaryprotocol(new TBinaryProtocol(membuffer));
  try {
    object->read(binaryprotocol.get());
  } catch(const TException &ex) {
                Log::WriteToDisk(ERROR, "%s", ex.what());
    return false;
  }
  return true;
}

template <typename T>
const std::string FromThriftToString(const T *object) {
  boost::shared_ptr<TMemoryBuffer> membuffer(new TMemoryBuffer());
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(membuffer));
  object->write(protocol.get());
  uint8* buffer = NULL;
  uint32 buffer_size = 0;
  membuffer->getBuffer(&buffer, &buffer_size);
  return std::string(reinterpret_cast<const char*>(buffer), buffer_size);
}

template <typename T>
const std::string FromThriftToDebugString(const T *object) {
  boost::shared_ptr<TMemoryBuffer> membuffer(new TMemoryBuffer());
  boost::shared_ptr<TProtocol> protocol(new TDebugProtocol(membuffer));
  object->write(protocol.get());
  uint8* buffer = NULL;
  uint32 buffer_size = 0;
  membuffer->getBuffer(&buffer, &buffer_size);
  return std::string(reinterpret_cast<const char*>(buffer), buffer_size);
}

template <typename T>
const std::string FromThriftToUtf8DebugString(const T *object) {
  std::string debug_string = FromThriftToDebugString(object);
  debug_string.append(" ");
  std::string out;
  for (size_t i = 0; i < debug_string.length();) {
    if (debug_string[i] == '\\') {
      switch (debug_string[i + 1]) {
        case 'x': {
              std::string hex_string;
              hex_string.insert(hex_string.end(), debug_string[i + 2]);
              hex_string.insert(hex_string.end(), debug_string[i + 3]);
              int hex_value = 0;
              HexStringToInt(hex_string, &hex_value);
              out.insert(out.end(), static_cast<unsigned char>(hex_value));
              i += 4;
              break;
            }
        case 'a': out.insert(out.end(), '\a'); i += 2; break;
        case 'b': out.insert(out.end(), '\b'); i += 2; break;
        case 'f': out.insert(out.end(), '\f'); i += 2; break;
        case 'n': out.insert(out.end(), '\n'); i += 2; break;
        case 'r': out.insert(out.end(), '\r'); i += 2; break;
        case 't': out.insert(out.end(), '\t'); i += 2; break;
        case 'v': out.insert(out.end(), '\v'); i += 2; break;
        case '\\': out.insert(out.end(), '\\'); i += 2; break;
        case '"': out.insert(out.end(), '"'); i += 2; break;
      }
    } else {
      out.insert(out.end(), debug_string[i]);
      i++;
    }
  }
  return out;
}
