#include <gtest/gtest.h>
#include <sms-manager.h>

TEST(SmsManagerTest, ParseGsmTime) {
    EXPECT_EQ(SmsManager::ParseGsmTime("\"24/05/01,12:34:56+08\""), "2024-05-01 12:34:56");
    EXPECT_EQ(SmsManager::ParseGsmTime("\"24/05/01,00:00:00+00\""), "2024-05-01 00:00:00");
}
