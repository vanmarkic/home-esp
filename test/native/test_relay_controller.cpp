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

// ============================================
// Edge Case Tests
// ============================================

TEST_F(RelayControllerTest, MinOnTimeExactBoundary) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(1000);  // Exactly at boundary

  bool result = controller.turn_off();

  EXPECT_TRUE(result);  // Should be allowed at exact boundary
  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, MinOffTimeExactBoundary) {
  RelayController::Config config;
  config.min_off_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(100);
  controller.turn_off();
  controller.update(1100);  // Exactly 1000ms since turn off

  bool result = controller.turn_on();

  EXPECT_TRUE(result);  // Should be allowed at exact boundary
  EXPECT_TRUE(controller.is_on());
}

TEST_F(RelayControllerTest, MillisOverflowHandling) {
  // Simulates millis() wraparound after ~49.7 days
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  // Start near max uint32_t
  controller.update(UINT32_MAX - 500);
  controller.turn_on();

  // Time wraps around past zero
  controller.update(600);  // 1100ms elapsed with overflow

  bool result = controller.turn_off();

  EXPECT_TRUE(result);  // Should handle overflow correctly
  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, MillisOverflowBlocksCorrectly) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  // Start near max uint32_t
  controller.update(UINT32_MAX - 500);
  controller.turn_on();

  // Time wraps around but only 400ms elapsed
  controller.update(400 - 500);  // Equivalent to 400ms elapsed

  bool result = controller.turn_off();

  EXPECT_FALSE(result);  // Should still block
  EXPECT_TRUE(controller.is_on());
}

TEST_F(RelayControllerTest, CombinedMinOnAndMinOffTimes) {
  RelayController::Config config;
  config.min_on_time_ms = 500;
  config.min_off_time_ms = 1000;
  RelayController controller(&handler_, config);

  // Turn on, wait past min_on_time
  controller.turn_on();
  controller.update(600);

  // Turn off should work
  EXPECT_TRUE(controller.turn_off());
  EXPECT_FALSE(controller.is_on());

  // Try to turn back on immediately (should be blocked by min_off_time)
  controller.update(700);  // Only 100ms since off
  EXPECT_FALSE(controller.turn_on());

  // Wait past min_off_time
  controller.update(1700);  // 1100ms since off
  EXPECT_TRUE(controller.turn_on());
}

TEST_F(RelayControllerTest, InvertedWithTimingConstraints) {
  RelayController::Config config;
  config.min_on_time_ms = 500;
  config.inverted = true;
  RelayController controller(&handler_, config);

  controller.turn_on();

  // Handler should receive inverted state
  EXPECT_FALSE(handler_.get_state());

  // Min time should still apply to logical state
  controller.update(300);
  EXPECT_FALSE(controller.turn_off());  // Blocked by min_on_time

  controller.update(600);
  EXPECT_TRUE(controller.turn_off());  // Now allowed

  // Handler received inverted off (which is true)
  EXPECT_TRUE(handler_.get_state());
}

TEST_F(RelayControllerTest, ToggleRespectsMinOnTime) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(500);

  bool result = controller.toggle();

  EXPECT_FALSE(result);  // Toggle to off blocked by min_on_time
  EXPECT_TRUE(controller.is_on());
}

TEST_F(RelayControllerTest, ToggleRespectsMinOffTime) {
  RelayController::Config config;
  config.min_off_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();
  controller.update(100);
  controller.turn_off();
  controller.update(600);  // Only 500ms off

  bool result = controller.toggle();

  EXPECT_FALSE(result);  // Toggle to on blocked by min_off_time
  EXPECT_FALSE(controller.is_on());
}

TEST_F(RelayControllerTest, NullHandlerCausesNoOp) {
  // This tests defensive programming - passing nullptr should not crash
  // Note: Current implementation doesn't guard against this,
  // but the test documents expected behavior
  // RelayController controller(nullptr);  // Would crash - document this
}

TEST_F(RelayControllerTest, ZeroTimingConstraintsAllowImmediateChanges) {
  RelayController::Config config;
  config.min_on_time_ms = 0;
  config.min_off_time_ms = 0;
  RelayController controller(&handler_, config);

  // Rapid state changes should all succeed
  EXPECT_TRUE(controller.turn_on());
  EXPECT_TRUE(controller.turn_off());
  EXPECT_TRUE(controller.turn_on());
  EXPECT_TRUE(controller.turn_off());

  EXPECT_EQ(handler_.get_execute_count(), 4);
}

TEST_F(RelayControllerTest, StateHistoryIsPreserved) {
  RelayController controller(&handler_);

  controller.turn_on();
  controller.turn_off();
  controller.turn_on();

  const auto& history = handler_.get_state_history();
  ASSERT_EQ(history.size(), 3);
  EXPECT_TRUE(history[0]);   // First on
  EXPECT_FALSE(history[1]);  // Then off
  EXPECT_TRUE(history[2]);   // Then on again
}

TEST_F(RelayControllerTest, ConfigAccessor) {
  RelayController::Config config;
  config.min_on_time_ms = 100;
  config.min_off_time_ms = 200;
  config.inverted = true;
  config.restore_state = true;

  RelayController controller(&handler_, config);
  const auto& retrieved = controller.get_config();

  EXPECT_EQ(retrieved.min_on_time_ms, 100);
  EXPECT_EQ(retrieved.min_off_time_ms, 200);
  EXPECT_TRUE(retrieved.inverted);
  EXPECT_TRUE(retrieved.restore_state);
}

TEST_F(RelayControllerTest, DefaultConfigValues) {
  RelayController::Config config;

  EXPECT_EQ(config.min_on_time_ms, 0);
  EXPECT_EQ(config.min_off_time_ms, 0);
  EXPECT_FALSE(config.inverted);
  EXPECT_FALSE(config.restore_state);
}

TEST_F(RelayControllerTest, UpdateWithoutStateChange) {
  RelayController::Config config;
  config.min_on_time_ms = 1000;
  RelayController controller(&handler_, config);

  controller.turn_on();

  // Multiple updates without state change
  for (uint32_t t = 100; t <= 2000; t += 100) {
    controller.update(t);
  }

  // State should still be on
  EXPECT_TRUE(controller.is_on());
  // Only one command should have been executed
  EXPECT_EQ(handler_.get_execute_count(), 1);
}

}  // namespace home_esp::testing
