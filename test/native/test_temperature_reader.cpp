// Unit tests for TemperatureReader

#include <gtest/gtest.h>
#include <cmath>

#include "core/temperature_reader.h"
#include "mocks/mock_sensor_publisher.h"

namespace home_esp::testing {

class TemperatureReaderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    publisher_.reset();
  }

  MockSensorPublisher publisher_;
};

TEST_F(TemperatureReaderTest, PublishesValidMidRangeTemperature) {
  TemperatureReader reader(&publisher_);

  // Mid-range ADC value (2048 out of 4095)
  // Should produce a temperature around mid-range
  reader.process_raw_reading(2048);

  ASSERT_EQ(publisher_.get_publish_count(), 1);
  EXPECT_EQ(publisher_.get_unavailable_count(), 0);

  float temp = publisher_.get_last_value();
  // With default config: -40 to 85°C range
  // Mid-point ADC should give roughly mid-point temp
  EXPECT_GT(temp, -40.0f);
  EXPECT_LT(temp, 85.0f);
}

TEST_F(TemperatureReaderTest, PublishesValidLowTemperature) {
  TemperatureReader reader(&publisher_);

  // Low ADC value - should be near minimum temperature
  reader.process_raw_reading(100);

  ASSERT_EQ(publisher_.get_publish_count(), 1);
  float temp = publisher_.get_last_value();
  EXPECT_LT(temp, 0.0f);  // Should be below zero
}

TEST_F(TemperatureReaderTest, PublishesValidHighTemperature) {
  TemperatureReader reader(&publisher_);

  // High ADC value - should be near maximum temperature
  reader.process_raw_reading(4000);

  ASSERT_EQ(publisher_.get_publish_count(), 1);
  float temp = publisher_.get_last_value();
  EXPECT_GT(temp, 60.0f);  // Should be warm
}

TEST_F(TemperatureReaderTest, PublishesUnavailableForOutOfRangeHigh) {
  TemperatureReader::Config config;
  config.max_valid_temp = 50.0f;  // Lower max for easier testing
  TemperatureReader reader(&publisher_, config);

  // Max ADC value - will produce temp above max_valid_temp
  reader.process_raw_reading(4095);

  EXPECT_EQ(publisher_.get_publish_count(), 0);
  EXPECT_EQ(publisher_.get_unavailable_count(), 1);
}

TEST_F(TemperatureReaderTest, AppliesCalibrationOffset) {
  TemperatureReader::Config config;
  config.offset = 2.5f;
  TemperatureReader reader(&publisher_, config);

  reader.process_raw_reading(2048);
  float with_offset = publisher_.get_last_value();

  publisher_.reset();
  config.offset = 0.0f;
  TemperatureReader reader_no_offset(&publisher_, config);
  reader_no_offset.process_raw_reading(2048);
  float without_offset = publisher_.get_last_value();

  EXPECT_NEAR(with_offset - without_offset, 2.5f, 0.01f);
}

TEST_F(TemperatureReaderTest, CanUpdateOffsetAfterConstruction) {
  TemperatureReader reader(&publisher_);

  reader.process_raw_reading(2048);
  float initial = publisher_.get_last_value();

  publisher_.reset();
  reader.set_offset(5.0f);
  reader.process_raw_reading(2048);
  float adjusted = publisher_.get_last_value();

  EXPECT_NEAR(adjusted - initial, 5.0f, 0.01f);
}

TEST_F(TemperatureReaderTest, HandlesCustomAdcResolution) {
  TemperatureReader::Config config;
  config.adc_resolution = 1023;  // 10-bit ADC
  TemperatureReader reader(&publisher_, config);

  // Mid-point of 10-bit ADC
  reader.process_raw_reading(512);

  ASSERT_EQ(publisher_.get_publish_count(), 1);
  float temp = publisher_.get_last_value();
  // Should still be mid-range temperature
  EXPECT_GT(temp, 10.0f);
  EXPECT_LT(temp, 40.0f);
}

TEST_F(TemperatureReaderTest, MultipleReadingsAllPublished) {
  TemperatureReader reader(&publisher_);

  reader.process_raw_reading(1000);
  reader.process_raw_reading(2000);
  reader.process_raw_reading(3000);

  EXPECT_EQ(publisher_.get_publish_count(), 3);

  const auto& values = publisher_.get_published_values();
  // Values should be increasing (more ADC = more temp)
  EXPECT_LT(values[0], values[1]);
  EXPECT_LT(values[1], values[2]);
}

}  // namespace home_esp::testing
