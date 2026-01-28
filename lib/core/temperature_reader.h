#pragma once

// TemperatureReader - Example business logic
// Pure C++ with no ESPHome dependencies
// Converts raw ADC readings to temperature and publishes via interface

#include "interfaces/i_sensor_publisher.h"
#include <cstdint>

namespace home_esp {

class TemperatureReader {
 public:
  /// Configuration for temperature conversion
  struct Config {
    float min_valid_temp{-40.0f};   // Minimum valid temperature (Celsius)
    float max_valid_temp{85.0f};    // Maximum valid temperature (Celsius)
    float adc_min_voltage{0.0f};    // ADC voltage at min temp
    float adc_max_voltage{3.3f};    // ADC voltage at max temp
    uint16_t adc_resolution{4095};  // ADC max value (12-bit = 4095)
    float offset{0.0f};             // Calibration offset

    Config() = default;
  };

  explicit TemperatureReader(ISensorPublisher* publisher, Config config = Config{})
      : publisher_(publisher), config_(config) {}

  /// Process a raw ADC reading
  void process_raw_reading(uint16_t raw_adc) {
    float celsius = convert_to_celsius(raw_adc);

    if (is_valid(celsius)) {
      publisher_->publish(celsius + config_.offset);
    } else {
      publisher_->publish_unavailable();
    }
  }

  /// Get the current configuration
  const Config& get_config() const { return config_; }

  /// Update calibration offset
  void set_offset(float offset) { config_.offset = offset; }

 private:
  float convert_to_celsius(uint16_t raw_adc) const {
    // Linear conversion from ADC to temperature
    // Assumes linear sensor like TMP36 or similar
    float voltage = (static_cast<float>(raw_adc) / config_.adc_resolution)
                    * config_.adc_max_voltage;

    // Map voltage range to temperature range
    float temp_range = config_.max_valid_temp - config_.min_valid_temp;
    float voltage_range = config_.adc_max_voltage - config_.adc_min_voltage;

    return config_.min_valid_temp +
           (voltage - config_.adc_min_voltage) * temp_range / voltage_range;
  }

  bool is_valid(float temp) const {
    return temp >= config_.min_valid_temp && temp <= config_.max_valid_temp;
  }

  ISensorPublisher* publisher_;
  Config config_;
};

}  // namespace home_esp
