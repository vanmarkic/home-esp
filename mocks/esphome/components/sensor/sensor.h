#pragma once

// ESPHome Sensor API Mock
// Lightweight stub for unit testing sensor components

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace sensor {

/// Sensor state classes (matching ESPHome)
enum StateClass : uint8_t {
  STATE_CLASS_NONE = 0,
  STATE_CLASS_MEASUREMENT = 1,
  STATE_CLASS_TOTAL_INCREASING = 2,
  STATE_CLASS_TOTAL = 3,
};

/// Base sensor class
class Sensor {
 public:
  virtual ~Sensor() = default;

  /// Publish a new state value
  void publish_state(float value) {
    raw_state = value;
    state = value;  // In real ESPHome, filters would be applied here
    has_state_ = true;
    published_values_.push_back(value);
  }

  /// Check if sensor has a valid state
  bool has_state() const { return has_state_; }

  /// Get the sensor's name
  const std::string& get_name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  /// Get/set unit of measurement
  const std::string& get_unit_of_measurement() const { return unit_; }
  void set_unit_of_measurement(const std::string& unit) { unit_ = unit; }

  /// Get/set accuracy decimals
  int8_t get_accuracy_decimals() const { return accuracy_decimals_; }
  void set_accuracy_decimals(int8_t decimals) { accuracy_decimals_ = decimals; }

  /// Get/set state class
  StateClass get_state_class() const { return state_class_; }
  void set_state_class(StateClass state_class) { state_class_ = state_class; }

  /// Get/set icon
  const std::string& get_icon() const { return icon_; }
  void set_icon(const std::string& icon) { icon_ = icon; }

  /// Current state (after filters)
  float state{NAN};

  /// Raw state (before filters)
  float raw_state{NAN};

  // ========== Test Helpers ==========

  /// Get all published values (for test assertions)
  const std::vector<float>& test_get_published_values() const {
    return published_values_;
  }

  /// Get last published value
  float test_get_last_value() const {
    if (published_values_.empty()) return NAN;
    return published_values_.back();
  }

  /// Get count of published values
  size_t test_get_publish_count() const { return published_values_.size(); }

  /// Clear published values (reset for next test)
  void test_reset() {
    published_values_.clear();
    state = NAN;
    raw_state = NAN;
    has_state_ = false;
  }

  /// Check if NAN was published (unavailable)
  bool test_was_unavailable_published() const {
    for (float v : published_values_) {
      if (std::isnan(v)) return true;
    }
    return false;
  }

 private:
  std::string name_;
  std::string unit_;
  std::string icon_;
  int8_t accuracy_decimals_{0};
  StateClass state_class_{STATE_CLASS_NONE};
  bool has_state_{false};
  std::vector<float> published_values_;
};

// Logging helper macro (used in ESPHome components)
#define LOG_SENSOR(prefix, type, sensor) \
  if (sensor != nullptr) { \
    ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, type, sensor->get_name().c_str()); \
  }

}  // namespace sensor
}  // namespace esphome
