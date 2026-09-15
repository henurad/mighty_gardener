#include <gtest/gtest.h>
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