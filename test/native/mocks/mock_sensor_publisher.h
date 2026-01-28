#pragma once

// MockSensorPublisher - Test double for ISensorPublisher

#include "core/interfaces/i_sensor_publisher.h"
#include <cmath>
#include <vector>

namespace home_esp::testing {

class MockSensorPublisher : public ISensorPublisher {
 public:
  void publish(float value) override {
    published_values_.push_back(value);
  }

  void publish_unavailable() override {
    unavailable_count_++;
  }

  // Test assertions
  const std::vector<float>& get_published_values() const {
    return published_values_;
  }

  float get_last_value() const {
    if (published_values_.empty()) return NAN;
    return published_values_.back();
  }

  size_t get_publish_count() const { return published_values_.size(); }
  int get_unavailable_count() const { return unavailable_count_; }

  void reset() {
    published_values_.clear();
    unavailable_count_ = 0;
  }

 private:
  std::vector<float> published_values_;
  int unavailable_count_{0};
};

}  // namespace home_esp::testing
