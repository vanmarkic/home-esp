// Unit tests for RelayController

#include <gtest/gtest.h>

#include "core/relay_controller.h"
#include "mocks/mock_command_handler.h"

namespace home_esp::testing {

class RelayControllerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_.reset();
  }

  MockCommandHandler handler_;
};

TEST_F(RelayControllerTest, StartsInOffState) {
  RelayController controller(&handler_);

  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, TurnOnExecutesCommand) {
  RelayController controller(&handler_);

  bool result = controller.turn_on();

  EXPECT_TRUE(result);
  EXPECT_TRUE(controller.is_on());
  EXPECT_EQ(handler_.get_execute_count(), 1);
  EXPECT_TRUE(handler_.get_state());
}

TEST_F(RelayControllerTest, TurnOffExecutesCommand) {
  RelayController controller(&handler_);

  controller.turn_on();
  handler_.reset();

  bool result = controller.turn_off();

  EXPECT_TRUE(result);
  EXPECT_FALSE(controller.is_on());
  EXPECT_EQ(handler_.get_execute_count(), 1);
  EXPECT_FALSE(handler_.get_state());
}

TEST_F(RelayControllerTest, ToggleFromOff) {
  RelayController controller(&handler_);

  bool result = controller.toggle();

  EXPECT_TRUE(result);
  EXPECT_TRUE(controller.is_on());
}

TEST_F(RelayControllerTest, ToggleFromOn) {
  RelayController controller(&handler_);

  controller.turn_on();
  bool result = controller.toggle();

  EXPECT_TRUE(result);
  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, MinOnTimeBlocksTurnOff) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(500);  // 500ms elapsed

  bool result = controller.turn_off();

  EXPECT_FALSE(result);  // Should be blocked
  EXPECT_TRUE(controller.is_on());  // Still on
}

TEST_F(RelayControllerTest, MinOnTimeAllowsTurnOffAfterElapsed) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(1500);  // 1500ms elapsed - past min time

  bool result = controller.turn_off();

  EXPECT_TRUE(result);
  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, MinOffTimeBlocksTurnOn) {
  RelayController::Config config;
  config.min_off_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(100);
  controller.turn_off();
  controller.update(600);  // Only 500ms since turn off

  bool result = controller.turn_on();

  EXPECT_FALSE(result);  // Should be blocked
  EXPECT_FALSE(controller.is_on());  // Still off
}

TEST_F(RelayControllerTest, InvertedModeTurnsOnWhenRequestedOff) {
  RelayController::Config config;
  config.inverted = true;
  RelayController controller(&handler_, config);

  controller.turn_on();

  // Internal state is "on", but output should be inverted
  EXPECT_TRUE(controller.is_on());
  // The handler received the inverted value (false)
  EXPECT_FALSE(handler_.get_state());
}

TEST_F(RelayControllerTest, InvertedModeTurnsOffWhenRequestedOn) {
  RelayController::Config config;
  config.inverted = true;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.turn_off();

  EXPECT_FALSE(controller.is_on());
  // The handler received the inverted value (true)
  EXPECT_TRUE(handler_.get_state());
}

TEST_F(RelayControllerTest, SameStateDoesNotBlock) {
  RelayController::Config config;
  config.min_on_time_ms = 10000;  // Very long min time
  RelayController controller(&handler_, config);

  controller.turn_on();

  // Calling turn_on again should succeed (no state change)
  bool result = controller.turn_on();

  EXPECT_TRUE(result);
}

TEST_F(RelayControllerTest, RapidToggleWithNoTimingConstraints) {
  RelayController controller(&handler_);

  for (int i = 0; i < 10; ++i) {
    controller.toggle();
  }

  EXPECT_EQ(handler_.get_execute_count(), 10);
  // After 10 toggles from off, should be off again
  EXPECT_FALSE(controller.is_on());
}

}  // namespace home_esp::testing
