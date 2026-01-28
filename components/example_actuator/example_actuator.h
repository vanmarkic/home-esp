#pragma once

/// @file example_actuator.h
/// @brief ESPHome component that wraps RelayController business logic
///
/// This component demonstrates the separation between ESPHome platform
/// bindings and testable C++ business logic. The RelayController handles
/// all state management and timing constraints, while this component
/// provides the ESPHome lifecycle integration.
///
/// ## Architecture
/// ```
/// Home Assistant <-> ESPHome Switch <-> ExampleActuatorComponent
///                                               |
///                                        RelayController (pure C++)
///                                               |
///                                        ESPHomeSwitchAdapter
/// ```
///
/// ## Configuration (YAML):
/// @code{.yaml}
/// switch:
///   - platform: example_actuator
///     name: "My Relay"
///     min_on_time: 5s    # Optional: minimum time to stay on
///     min_off_time: 10s  # Optional: minimum time to stay off
///     inverted: false    # Optional: invert output logic
/// @endcode
///
/// ## Testing
/// The RelayController can be tested independently using MockCommandHandler,
/// enabling fast native unit tests without ESPHome dependencies.

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

// Include our abstracted business logic
#include "core/relay_controller.h"
#include "core/adapters/esphome_switch_adapter.h"

#include <memory>

namespace home_esp {

static const char* const ACTUATOR_TAG = "example_actuator";

// Forward declaration
class ExampleSwitch;

class ExampleActuatorComponent : public esphome::Component {
 public:
  ExampleActuatorComponent() = default;

  // ESPHome configuration setters
  void set_switch(ExampleSwitch* switch_obj) { switch_ = switch_obj; }
  void set_min_on_time(uint32_t ms) { min_on_time_ms_ = ms; }
  void set_min_off_time(uint32_t ms) { min_off_time_ms_ = ms; }
  void set_inverted(bool inverted) { inverted_ = inverted; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_setup_priority() const override {
    return esphome::setup_priority::DATA;
  }

  // Called by the switch to request state change
  bool request_state(bool state);

 private:
  ExampleSwitch* switch_{nullptr};
  uint32_t min_on_time_ms_{0};
  uint32_t min_off_time_ms_{0};
  bool inverted_{false};

  std::unique_ptr<ESPHomeSwitchAdapter> adapter_;
  std::unique_ptr<RelayController> controller_;
};

/// The actual switch that appears in Home Assistant
class ExampleSwitch : public esphome::switch_::Switch, public esphome::Component {
 public:
  void set_parent(ExampleActuatorComponent* parent) { parent_ = parent; }

  void setup() override {}

  void dump_config() override {
    esphome::switch_::log_switch(ACTUATOR_TAG, "  ", "Switch", this);
  }

 protected:
  void write_state(bool state) override {
    if (parent_ != nullptr) {
      if (parent_->request_state(state)) {
        publish_state(state);
      }
    }
  }

 private:
  ExampleActuatorComponent* parent_{nullptr};
};

// Implementation of ExampleActuatorComponent methods
inline void ExampleActuatorComponent::setup() {
  ESP_LOGCONFIG(ACTUATOR_TAG, "Setting up Example Actuator...");

  if (switch_ != nullptr) {
    switch_->set_parent(this);

    // Create adapter and controller
    adapter_ = std::make_unique<ESPHomeSwitchAdapter>(switch_);

    RelayController::Config config;
    config.min_on_time_ms = min_on_time_ms_;
    config.min_off_time_ms = min_off_time_ms_;
    config.inverted = inverted_;

    controller_ = std::make_unique<RelayController>(adapter_.get(), config);
  }
}

inline void ExampleActuatorComponent::loop() {
  if (controller_ != nullptr) {
    controller_->update(millis());
  }
}

inline void ExampleActuatorComponent::dump_config() {
  ESP_LOGCONFIG(ACTUATOR_TAG, "Example Actuator:");
  ESP_LOGCONFIG(ACTUATOR_TAG, "  Min ON time: %u ms", min_on_time_ms_);
  ESP_LOGCONFIG(ACTUATOR_TAG, "  Min OFF time: %u ms", min_off_time_ms_);
  ESP_LOGCONFIG(ACTUATOR_TAG, "  Inverted: %s", inverted_ ? "YES" : "NO");
}

inline bool ExampleActuatorComponent::request_state(bool state) {
  if (controller_ == nullptr) {
    return false;
  }
  return state ? controller_->turn_on() : controller_->turn_off();
}

}  // namespace home_esp
