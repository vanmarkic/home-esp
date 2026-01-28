#pragma once

// IProtocolCodec Interface
// Abstraction for encoding/decoding protocol messages (RF433, IR, etc.)
// Allows business logic to be tested without ESPHome dependencies

#include <cstddef>
#include <cstdint>

namespace home_esp {

/// Decoded message structure
struct DecodedMessage {
  uint32_t code{0};          // The decoded value/code
  uint8_t protocol{0};       // Protocol identifier
  uint16_t bit_length{0};    // Number of bits in the code
  bool valid{false};         // Whether decoding was successful
};

/// Protocol codec interface
class IProtocolCodec {
 public:
  virtual ~IProtocolCodec() = default;

  /// Decode raw data into a message
  /// @param data Raw input data (timing pulses, bytes, etc.)
  /// @param len Length of input data
  /// @param out Output decoded message
  /// @return true if decoding succeeded
  virtual bool decode(const uint8_t* data, size_t len, DecodedMessage& out) = 0;

  /// Encode a message into raw data
  /// @param msg Message to encode
  /// @param out Output buffer for encoded data
  /// @param len Input: buffer size, Output: actual encoded length
  /// @return true if encoding succeeded
  virtual bool encode(const DecodedMessage& msg, uint8_t* out, size_t& len) = 0;

  /// Get protocol name for logging
  virtual const char* get_protocol_name() const = 0;
};

}  // namespace home_esp
