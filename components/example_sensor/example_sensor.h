#pragma once

// ExampleSensorComponent
// ESPHome component that wraps TemperatureReader business logic

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

// Include our abstracted business logic
#include "core/temperature_reader.h"
#include "core/adapters/esphome_sensor_adapter.h"

#include <memory>

namespace home_esp {

static const char* const TAG = "example_sensor";

class ExampleSensorComponent : public esphome::PollingComponent {
 public:
  ExampleSensorComponent() = default;

  // ESPHome configuration setters
  void set_sensor(esphome::sensor::Sensor* sensor) { sensor_ = sensor; }
  void set_offset(float offset) { offset_ = offset; }
  void set_min_temperature(float min_temp) { min_temp_ = min_temp; }
  void set_max_temperature(float max_temp) { max_temp_ = max_temp; }

  void setup() override {
    ESP_LOGCONFIG(TAG, "Setting up Example Sensor...");

    // Create the adapter that bridges our interface to ESPHome
    adapter_ = std::make_unique<ESPHomeSensorAdapter>(sensor_);

    // Configure and create the business logic
    TemperatureReader::Config config;
    config.offset = offset_;
    config.min_valid_temp = min_temp_;
    config.max_valid_temp = max_temp_;

    reader_ = std::make_unique<TemperatureReader>(adapter_.get(), config);
  }

  void update() override {
    // In a real component, this would read from actual hardware (ADC, I2C, etc.)
    // For this example, we simulate a reading
    uint16_t raw_adc = read_adc_value();

    reader_->process_raw_reading(raw_adc);
  }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "Example Sensor:");
    ESP_LOGCONFIG(TAG, "  Offset: %.1f°C", offset_);
    ESP_LOGCONFIG(TAG, "  Valid range: %.1f°C to %.1f°C", min_temp_, max_temp_);
    LOG_SENSOR("  ", "Temperature", sensor_);
  }

  float get_setup_priority() const override {
    return esphome::setup_priority::DATA;
  }

 protected:
  // Simulate reading from ADC - override this for real hardware
  virtual uint16_t read_adc_value() {
    // Simulate a temperature around 25°C
    // In a real component, this would be analogRead() or I2C read
    return 2048;  // Mid-range ADC value
  }

 private:
  esphome::sensor::Sensor* sensor_{nullptr};
  float offset_{0.0f};
  float min_temp_{-40.0f};
  float max_temp_{85.0f};

  std::unique_ptr<ESPHomeSensorAdapter> adapter_;
  std::unique_ptr<TemperatureReader> reader_;
};

}  // namespace home_esp
