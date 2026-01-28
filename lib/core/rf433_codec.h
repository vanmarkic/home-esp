#pragma once

// RF433Codec - Example protocol bridge logic
// Pure C++ with no ESPHome dependencies
// Decodes and encodes RF433 signals (simplified example)

#include "interfaces/i_protocol_codec.h"
#include "interfaces/i_binary_publisher.h"
#include <cstring>

namespace home_esp {

/// Simple RF433 protocol codec
/// Supports basic fixed-code protocols (like PT2262)
class RF433Codec : public IProtocolCodec {
 public:
  // Protocol identifiers
  static constexpr uint8_t PROTOCOL_PT2262 = 1;
  static constexpr uint8_t PROTOCOL_EV1527 = 2;

  /// Timing configuration for the protocol
  struct TimingConfig {
    uint16_t pulse_length_us{350};    // Base pulse length in microseconds
    uint8_t sync_high_pulses{1};      // Number of high pulses in sync
    uint8_t sync_low_pulses{31};      // Number of low pulses in sync
    uint8_t zero_high_pulses{1};      // High pulses for '0' bit
    uint8_t zero_low_pulses{3};       // Low pulses for '0' bit
    uint8_t one_high_pulses{3};       // High pulses for '1' bit
    uint8_t one_low_pulses{1};        // Low pulses for '1' bit
    uint8_t tolerance_percent{25};    // Timing tolerance
  };

  explicit RF433Codec(TimingConfig config = {}) : config_(config) {}

  bool decode(const uint8_t* data, size_t len, DecodedMessage& out) override {
    // Simplified decoding: expects raw pulse timings as uint16_t pairs
    // Format: [high_us, low_us, high_us, low_us, ...]

    if (len < 4 || len % 4 != 0) {
      out.valid = false;
      return false;
    }

    // Number of pulse pairs
    size_t num_pairs = len / 4;
    if (num_pairs < 2) {  // Need at least sync + 1 bit
      out.valid = false;
      return false;
    }

    // Cast to uint16_t pairs
    const uint16_t* pulses = reinterpret_cast<const uint16_t*>(data);

    // Check for sync pulse (first pair)
    uint16_t sync_high = pulses[0];
    uint16_t sync_low = pulses[1];

    if (!is_sync_pulse(sync_high, sync_low)) {
      out.valid = false;
      return false;
    }

    // Decode data bits
    uint32_t code = 0;
    uint16_t bits = 0;

    for (size_t i = 1; i < num_pairs && bits < 24; ++i) {
      uint16_t high_us = pulses[i * 2];
      uint16_t low_us = pulses[i * 2 + 1];

      int bit = decode_bit(high_us, low_us);
      if (bit < 0) {
        // Invalid timing, stop decoding
        break;
      }

      code = (code << 1) | bit;
      bits++;
    }

    if (bits < 8) {
      out.valid = false;
      return false;
    }

    out.code = code;
    out.protocol = PROTOCOL_PT2262;
    out.bit_length = bits;
    out.valid = true;

    return true;
  }

  bool encode(const DecodedMessage& msg, uint8_t* out, size_t& len) override {
    // Calculate required buffer size
    // Sync + data bits, each as uint16_t pair
    size_t pairs_needed = 1 + msg.bit_length;  // sync + bits
    size_t bytes_needed = pairs_needed * 4;    // Each pair is 4 bytes

    if (len < bytes_needed) {
      return false;
    }

    uint16_t* pulses = reinterpret_cast<uint16_t*>(out);
    size_t idx = 0;

    // Sync pulse
    pulses[idx++] = config_.pulse_length_us * config_.sync_high_pulses;
    pulses[idx++] = config_.pulse_length_us * config_.sync_low_pulses;

    // Data bits (MSB first)
    for (int i = msg.bit_length - 1; i >= 0; --i) {
      bool bit = (msg.code >> i) & 1;
      if (bit) {
        pulses[idx++] = config_.pulse_length_us * config_.one_high_pulses;
        pulses[idx++] = config_.pulse_length_us * config_.one_low_pulses;
      } else {
        pulses[idx++] = config_.pulse_length_us * config_.zero_high_pulses;
        pulses[idx++] = config_.pulse_length_us * config_.zero_low_pulses;
      }
    }

    len = idx * 2;  // Convert to bytes
    return true;
  }

  const char* get_protocol_name() const override {
    return "RF433/PT2262";
  }

  /// Get timing configuration
  const TimingConfig& get_config() const { return config_; }

 private:
  bool is_sync_pulse(uint16_t high_us, uint16_t low_us) const {
    uint16_t expected_high = config_.pulse_length_us * config_.sync_high_pulses;
    uint16_t expected_low = config_.pulse_length_us * config_.sync_low_pulses;

    return is_within_tolerance(high_us, expected_high) &&
           is_within_tolerance(low_us, expected_low);
  }

  int decode_bit(uint16_t high_us, uint16_t low_us) const {
    // Check for '1' bit
    uint16_t one_high = config_.pulse_length_us * config_.one_high_pulses;
    uint16_t one_low = config_.pulse_length_us * config_.one_low_pulses;

    if (is_within_tolerance(high_us, one_high) &&
        is_within_tolerance(low_us, one_low)) {
      return 1;
    }

    // Check for '0' bit
    uint16_t zero_high = config_.pulse_length_us * config_.zero_high_pulses;
    uint16_t zero_low = config_.pulse_length_us * config_.zero_low_pulses;

    if (is_within_tolerance(high_us, zero_high) &&
        is_within_tolerance(low_us, zero_low)) {
      return 0;
    }

    return -1;  // Invalid
  }

  bool is_within_tolerance(uint16_t actual, uint16_t expected) const {
    uint16_t margin = expected * config_.tolerance_percent / 100;
    return actual >= (expected - margin) && actual <= (expected + margin);
  }

  TimingConfig config_;
};

/// RF433 receiver that decodes signals and publishes events
class RF433Receiver {
 public:
  RF433Receiver(IProtocolCodec* codec, IBinaryPublisher* motion_publisher)
      : codec_(codec), motion_publisher_(motion_publisher) {}

  /// Process received pulse data
  void process_pulses(const uint8_t* data, size_t len) {
    DecodedMessage msg;
    if (codec_->decode(data, len, msg)) {
      last_code_ = msg.code;
      last_valid_ = true;

      // Example: treat certain codes as motion detection
      if (is_motion_code(msg.code)) {
        motion_publisher_->publish(true);
      }
    }
  }

  /// Get the last decoded code
  uint32_t get_last_code() const { return last_code_; }
  bool has_valid_code() const { return last_valid_; }

  /// Register a code as a motion sensor code
  void register_motion_code(uint32_t code) { motion_code_ = code; }

 private:
  bool is_motion_code(uint32_t code) const {
    return code == motion_code_;
  }

  IProtocolCodec* codec_;
  IBinaryPublisher* motion_publisher_;
  uint32_t last_code_{0};
  uint32_t motion_code_{0};
  bool last_valid_{false};
};

}  // namespace home_esp
