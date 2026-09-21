#include <gtest/gtest.h>
#include <sms-manager.h>
#include <sms-utils.h>

TEST(SmsUtilsTest, ValidatePhoneNumber) {
    SmsModule sms_module;

    EXPECT_FALSE(sms_module.ValidatePhoneNumber("hello world"));
    EXPECT_FALSE(sms_module.ValidatePhoneNumber("09133333333"));
    EXPECT_FALSE(sms_module.ValidatePhoneNumber("922920920"));
    EXPECT_FALSE(sms_module.ValidatePhoneNumber("+989131231"));
    EXPECT_FALSE(sms_module.ValidatePhoneNumber("+989131231212"));
    EXPECT_TRUE(sms_module.ValidatePhoneNumber("+989027732097"));
}

TEST(SmsManagerTest, ParseGsmTime) {
    EXPECT_EQ(SmsManager::ParseGsmTime("+CCLK: \"24/05/01,12:34:56+08\""), "2024-05-01 12:34:56");
    EXPECT_EQ(SmsManager::ParseGsmTime("+CCLK: \"25/12/24,00:00:00+08\""), "2025-12-24 00:00:00");
    EXPECT_TRUE(SmsManager::ParseGsmTime("+CCLK: \"bad\"").empty());
}