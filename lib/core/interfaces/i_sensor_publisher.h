#pragma once

// ISensorPublisher Interface
// Abstraction for publishing sensor values
// Allows business logic to be tested without ESPHome dependencies

namespace home_esp {

class ISensorPublisher {
 public:
  virtual ~ISensorPublisher() = default;

  /// Publish a sensor value
  virtual void publish(float value) = 0;

  /// Publish unavailable state (NAN in ESPHome)
  virtual void publish_unavailable() = 0;
};

}  // namespace home_esp
