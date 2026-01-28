#pragma once

// ESPHomeBinaryAdapter
// Bridges IBinaryPublisher interface to ESPHome's BinarySensor class

#ifdef UNIT_TEST
#include "esphome.h"
#else
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#include "interfaces/i_binary_publisher.h"

namespace home_esp {

class ESPHomeBinaryAdapter : public IBinaryPublisher {
 public:
  explicit ESPHomeBinaryAdapter(esphome::binary_sensor::BinarySensor* sensor)
      : sensor_(sensor) {}

  void publish(bool state) override {
    if (sensor_ != nullptr) {
      sensor_->publish_state(state);
    }
  }

 private:
  esphome::binary_sensor::BinarySensor* sensor_;
};

}  // namespace home_esp
