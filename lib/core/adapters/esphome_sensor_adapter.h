#pragma once

// ESPHomeSensorAdapter
// Bridges ISensorPublisher interface to ESPHome's Sensor class

#ifdef UNIT_TEST
#include "esphome.h"
#else
#include "esphome/components/sensor/sensor.h"
#endif

#include "interfaces/i_sensor_publisher.h"
#include <cmath>

namespace home_esp {

class ESPHomeSensorAdapter : public ISensorPublisher {
 public:
  explicit ESPHomeSensorAdapter(esphome::sensor::Sensor* sensor)
      : sensor_(sensor) {}

  void publish(float value) override {
    if (sensor_ != nullptr) {
      sensor_->publish_state(value);
    }
  }

  void publish_unavailable() override {
    if (sensor_ != nullptr) {
      sensor_->publish_state(NAN);
    }
  }

 private:
  esphome::sensor::Sensor* sensor_;
};

}  // namespace home_esp
