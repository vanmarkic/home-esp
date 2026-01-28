#pragma once

// MockBinaryPublisher - Test double for IBinaryPublisher

#include "core/interfaces/i_binary_publisher.h"
#include <vector>

namespace home_esp::testing {

class MockBinaryPublisher : public IBinaryPublisher {
 public:
  void publish(bool state) override {
    current_state_ = state;
    state_history_.push_back(state);
  }

  // Test assertions
  bool get_state() const { return current_state_; }

  const std::vector<bool>& get_state_history() const {
    return state_history_;
  }

  size_t get_publish_count() const { return state_history_.size(); }

  int count_true() const {
    int count = 0;
    for (bool v : state_history_) {
      if (v) count++;
    }
    return count;
  }

  void reset() {
    state_history_.clear();
    current_state_ = false;
  }

 private:
  bool current_state_{false};
  std::vector<bool> state_history_;
};

}  // namespace home_esp::testing
