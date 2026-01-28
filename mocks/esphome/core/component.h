#pragma once

// ESPHome Component API Mocks
// Lightweight stubs for unit testing without ESPHome dependencies

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace esphome {

// Setup priorities (matching ESPHome)
namespace setup_priority {
constexpr float BUS = 1000.0f;
constexpr float IO = 900.0f;
constexpr float HARDWARE = 800.0f;
constexpr float DATA = 600.0f;
constexpr float PROCESSOR = 400.0f;
constexpr float BLUETOOTH = 350.0f;
constexpr float AFTER_BLUETOOTH = 300.0f;
constexpr float WIFI = 250.0f;
constexpr float ETHERNET = 250.0f;
constexpr float BEFORE_CONNECTION = 220.0f;
constexpr float AFTER_WIFI = 200.0f;
constexpr float AFTER_CONNECTION = 100.0f;
constexpr float LATE = -100.0f;
}  // namespace setup_priority

/// Base class for all ESPHome components
class Component {
 public:
  virtual ~Component() = default;

  /// Called once to set up the component
  virtual void setup() {}

  /// Called repeatedly in the main loop
  virtual void loop() {}

  /// Called to dump configuration to logs
  virtual void dump_config() {}

  /// Get the setup priority of this component
  virtual float get_setup_priority() const { return setup_priority::DATA; }

  /// Mark this component as failed
  void mark_failed() { failed_ = true; }

  /// Check if this component has failed
  bool is_failed() const { return failed_; }

  /// Check if this component is ready
  bool is_ready() const { return ready_; }

  /// Get component status for diagnostics
  virtual void get_status(std::string& status) const { status = "OK"; }

  // Test helpers
  bool test_was_setup_called() const { return setup_called_; }
  int test_get_loop_count() const { return loop_count_; }

 protected:
  void set_ready() { ready_ = true; }

  // For tracking in tests
  mutable bool setup_called_{false};
  mutable int loop_count_{0};

 private:
  bool failed_{false};
  bool ready_{false};
};

/// Component that polls at a regular interval
class PollingComponent : public Component {
 public:
  explicit PollingComponent() : update_interval_(15000) {}
  explicit PollingComponent(uint32_t update_interval)
      : update_interval_(update_interval) {}

  /// Called at the configured interval
  virtual void update() = 0;

  /// Set the update interval in milliseconds
  void set_update_interval(uint32_t interval) { update_interval_ = interval; }

  /// Get the update interval in milliseconds
  uint32_t get_update_interval() const { return update_interval_; }

  float get_setup_priority() const override { return setup_priority::DATA; }

  // Test helper: simulate update cycle
  void test_trigger_update() {
    update_count_++;
    update();
  }

  int test_get_update_count() const { return update_count_; }

 protected:
  uint32_t update_interval_;
  int update_count_{0};
};

// Logging macros (no-ops for tests, can be overridden to capture)
#ifndef ESP_LOGD
#define ESP_LOGD(tag, ...) ((void)0)
#endif

#ifndef ESP_LOGI
#define ESP_LOGI(tag, ...) ((void)0)
#endif

#ifndef ESP_LOGW
#define ESP_LOGW(tag, ...) ((void)0)
#endif

#ifndef ESP_LOGE
#define ESP_LOGE(tag, ...) ((void)0)
#endif

#ifndef ESP_LOGCONFIG
#define ESP_LOGCONFIG(tag, ...) ((void)0)
#endif

#ifndef ESP_LOGV
#define ESP_LOGV(tag, ...) ((void)0)
#endif

// Optional macro (ESPHome uses this)
#define optional std::optional

}  // namespace esphome
