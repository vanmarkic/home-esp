#pragma once

// ExampleBridgeComponent
// ESPHome component for RF433 protocol bridging

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

// Include our abstracted business logic
#include "core/rf433_codec.h"
#include "core/adapters/esphome_binary_adapter.h"

#include <memory>

namespace home_esp {

static const char* const BRIDGE_TAG = "example_bridge";

class ExampleBridgeComponent : public esphome::Component {
 public:
  ExampleBridgeComponent() = default;

  // ESPHome configuration setters
  void set_motion_sensor(esphome::binary_sensor::BinarySensor* sensor) {
    motion_sensor_ = sensor;
  }
  void set_pulse_length(uint16_t length) { pulse_length_ = length; }
  void set_tolerance(uint8_t tolerance) { tolerance_ = tolerance; }
  void set_motion_code(uint32_t code) { motion_code_ = code; }

  void setup() override {
    ESP_LOGCONFIG(BRIDGE_TAG, "Setting up RF433 Bridge...");

    // Configure the codec
    RF433Codec::TimingConfig config;
    config.pulse_length_us = pulse_length_;
    config.tolerance_percent = tolerance_;

    codec_ = std::make_unique<RF433Codec>(config);

    // Create adapter for binary sensor
    if (motion_sensor_ != nullptr) {
      motion_adapter_ = std::make_unique<ESPHomeBinaryAdapter>(motion_sensor_);
      receiver_ = std::make_unique<RF433Receiver>(codec_.get(), motion_adapter_.get());
      receiver_->register_motion_code(motion_code_);
    }
  }

  void loop() override {
    // In a real component, this would read from an RF receiver
    // connected via GPIO interrupt or polling

    // Example: check for received data
    if (has_pending_rf_data()) {
      uint8_t buffer[256];
      size_t len = read_rf_data(buffer, sizeof(buffer));

      if (len > 0 && receiver_ != nullptr) {
        receiver_->process_pulses(buffer, len);

        if (receiver_->has_valid_code()) {
          ESP_LOGD(BRIDGE_TAG, "Received RF code: 0x%08X",
                   receiver_->get_last_code());
        }
      }
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(BRIDGE_TAG, "RF433 Bridge:");
    ESP_LOGCONFIG(BRIDGE_TAG, "  Pulse length: %u us", pulse_length_);
    ESP_LOGCONFIG(BRIDGE_TAG, "  Tolerance: %u%%", tolerance_);
    ESP_LOGCONFIG(BRIDGE_TAG, "  Motion code: 0x%08X", motion_code_);
    LOG_BINARY_SENSOR("  ", "Motion", motion_sensor_);
  }

  float get_setup_priority() const override {
    return esphome::setup_priority::DATA;
  }

  /// Manually inject RF data for testing
  void inject_rf_data(const uint8_t* data, size_t len) {
    if (receiver_ != nullptr) {
      receiver_->process_pulses(data, len);
    }
  }

  /// Get the last received code
  uint32_t get_last_code() const {
    return receiver_ ? receiver_->get_last_code() : 0;
  }

 protected:
  // Override these in subclass for real hardware
  virtual bool has_pending_rf_data() { return false; }
  virtual size_t read_rf_data(uint8_t* buffer, size_t max_len) {
    (void)buffer;
    (void)max_len;
    return 0;
  }

 private:
  esphome::binary_sensor::BinarySensor* motion_sensor_{nullptr};
  uint16_t pulse_length_{350};
  uint8_t tolerance_{25};
  uint32_t motion_code_{0};

  std::unique_ptr<RF433Codec> codec_;
  std::unique_ptr<ESPHomeBinaryAdapter> motion_adapter_;
  std::unique_ptr<RF433Receiver> receiver_;
};

}  // namespace home_esp
