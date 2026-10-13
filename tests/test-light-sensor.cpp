#include <gtest/gtest.h>

#include "hardware/light-sensor.h"

TEST(LightSensorTest, RawToPercentMapsCalibrationPoints) {
  EXPECT_FLOAT_EQ(LightSensor::RawToPercent(0, 0, 1023), 0.0f);
  EXPECT_FLOAT_EQ(LightSensor::RawToPercent(1023, 0, 1023), 100.0f);
  EXPECT_NEAR(LightSensor::RawToPercent(512, 0, 1023), 50.0489f, 0.001f);
}

TEST(LightSensorTest, RawToPercentClampsOutOfRangeValues) {
  EXPECT_FLOAT_EQ(LightSensor::RawToPercent(-500, 0, 1023), 0.0f);
  EXPECT_FLOAT_EQ(LightSensor::RawToPercent(2000, 0, 1023), 100.0f);
}

TEST(LightSensorTest, RawToPercentHandlesDegenerateCalibration) {
  EXPECT_FLOAT_EQ(LightSensor::RawToPercent(512, 500, 500), 0.0f);
}

TEST(LightSensorTest, ParsesPrefixedLine) {
  LightSensor sensor(nullptr, 0, 1023);

  sensor.ParseData("light:512\n");

  int raw = 0;
  float percent = 0.0f;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 512);
  ASSERT_TRUE(sensor.Read(percent));
  EXPECT_NEAR(percent, 50.0489f, 0.001f);
}

TEST(LightSensorTest, ParsesBareNumber) {
  LightSensor sensor(nullptr, 0, 1023);

  sensor.ParseData("300\r\n");

  int raw = 0;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 300);
}

TEST(LightSensorTest, BuffersPartialLines) {
  LightSensor sensor(nullptr, 0, 1023);

  sensor.ParseData("light:51");
  int raw = 0;
  EXPECT_FALSE(sensor.ReadRaw(raw));

  sensor.ParseData("2\n");
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 512);
}

TEST(LightSensorTest, ProcessesMultipleLinesInOneChunk) {
  LightSensor sensor(nullptr, 0, 1023);

  sensor.ParseData("light:100\nlight:900\n");

  int raw = 0;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 900);
}

TEST(LightSensorTest, IgnoresInvalidData) {
  LightSensor sensor(nullptr, 0, 1023);

  sensor.ParseData("garbage\n");

  int raw = 0;
  float percent = 0.0f;
  EXPECT_FALSE(sensor.ReadRaw(raw));
  EXPECT_FALSE(sensor.Read(percent));
}
