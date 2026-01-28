#pragma once

// ESPHomeSwitchAdapter
// Bridges ICommandHandler interface to ESPHome's Switch class

#ifdef UNIT_TEST
#include "esphome.h"
#else
#include "esphome/components/switch/switch.h"
#endif

#include "interfaces/i_command_handler.h"

namespace home_esp {

class ESPHomeSwitchAdapter : public ICommandHandler {
 public:
  explicit ESPHomeSwitchAdapter(esphome::switch_::Switch* switch_obj)
      : switch_(switch_obj) {}

  void execute(bool state) override {
    if (switch_ != nullptr) {
      if (state) {
        switch_->turn_on();
      } else {
        switch_->turn_off();
      }
      current_state_ = state;
    }
  }

  bool get_state() const override {
    return current_state_;
  }

 private:
  esphome::switch_::Switch* switch_;
  bool current_state_{false};
};

}  // namespace home_esp
