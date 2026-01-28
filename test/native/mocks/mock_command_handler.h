#pragma once

// MockCommandHandler - Test double for ICommandHandler

#include "core/interfaces/i_command_handler.h"
#include <vector>

namespace home_esp::testing {

class MockCommandHandler : public ICommandHandler {
 public:
  void execute(bool state) override {
    current_state_ = state;
    state_history_.push_back(state);
  }

  bool get_state() const override {
    return current_state_;
  }

  // Test assertions
  const std::vector<bool>& get_state_history() const {
    return state_history_;
  }

  size_t get_execute_count() const { return state_history_.size(); }

  void reset() {
    state_history_.clear();
    current_state_ = false;
  }

 private:
  bool current_state_{false};
  std::vector<bool> state_history_;
};

}  // namespace home_esp::testing
