#pragma once

// IBinaryPublisher Interface
// Abstraction for publishing binary sensor values
// Allows business logic to be tested without ESPHome dependencies

namespace home_esp {

class IBinaryPublisher {
 public:
  virtual ~IBinaryPublisher() = default;

  /// Publish a binary state (on/off, true/false)
  virtual void publish(bool state) = 0;
};

}  // namespace home_esp
