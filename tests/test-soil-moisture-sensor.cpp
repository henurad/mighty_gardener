#include <gtest/gtest.h>

#include "hardware/soil-moisture-sensor.h"

TEST(SoilMoistureSensorTest, RawToPercentMapsCalibrationPoints) {
  EXPECT_FLOAT_EQ(SoilMoistureSensor::RawToPercent(1023, 1023, 0), 0.0f);
  EXPECT_FLOAT_EQ(SoilMoistureSensor::RawToPercent(0, 1023, 0), 100.0f);
  EXPECT_NEAR(SoilMoistureSensor::RawToPercent(512, 1023, 0), 49.9511f, 0.001f);
}

TEST(SoilMoistureSensorTest, RawToPercentClampsOutOfRangeValues) {
  EXPECT_FLOAT_EQ(SoilMoistureSensor::RawToPercent(2000, 1023, 0), 0.0f);
  EXPECT_FLOAT_EQ(SoilMoistureSensor::RawToPercent(-500, 1023, 0), 100.0f);
}

TEST(SoilMoistureSensorTest, RawToPercentHandlesDegenerateCalibration) {
  EXPECT_FLOAT_EQ(SoilMoistureSensor::RawToPercent(512, 500, 500), 0.0f);
}

TEST(SoilMoistureSensorTest, ParsesPrefixedLine) {
  SoilMoistureSensor sensor(nullptr, 1023, 0);

  sensor.ParseData("soil:512\n");

  int raw = 0;
  float percent = 0.0f;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 512);
  ASSERT_TRUE(sensor.Read(percent));
  EXPECT_NEAR(percent, 49.9511f, 0.001f);
}

TEST(SoilMoistureSensorTest, ParsesBareNumber) {
  SoilMoistureSensor sensor(nullptr, 1023, 0);

  sensor.ParseData("300\r\n");

  int raw = 0;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 300);
}

TEST(SoilMoistureSensorTest, BuffersPartialLines) {
  SoilMoistureSensor sensor(nullptr, 1023, 0);

  sensor.ParseData("soil:51");
  int raw = 0;
  EXPECT_FALSE(sensor.ReadRaw(raw));

  sensor.ParseData("2\n");
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 512);
}

TEST(SoilMoistureSensorTest, ProcessesMultipleLinesInOneChunk) {
  SoilMoistureSensor sensor(nullptr, 1023, 0);

  sensor.ParseData("soil:100\nsoil:900\n");

  int raw = 0;
  ASSERT_TRUE(sensor.ReadRaw(raw));
  EXPECT_EQ(raw, 900);
}

TEST(SoilMoistureSensorTest, IgnoresInvalidData) {
  SoilMoistureSensor sensor(nullptr, 1023, 0);

  sensor.ParseData("garbage\n");

  int raw = 0;
  float percent = 0.0f;
  EXPECT_FALSE(sensor.ReadRaw(raw));
  EXPECT_FALSE(sensor.Read(percent));
}
