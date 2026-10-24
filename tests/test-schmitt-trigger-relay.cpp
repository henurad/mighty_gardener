#include <gtest/gtest.h>

#include <stdexcept>

#include "control/schmitt-trigger-relay.h"
#include "hardware/relay.h"

TEST(SchmittTriggerRelayTest, StartsOffBeforeUpdate) {
  Relay relay(17, false);
  SchmittTriggerRelay controller(relay, 30.0f, 70.0f);

  EXPECT_FALSE(controller.IsOn());
}

TEST(SchmittTriggerRelayTest, TurnsOnAtOrBelowMin) {
  Relay relay(17, false);
  SchmittTriggerRelay controller(relay, 30.0f, 70.0f);

  controller.Update(30.0f);
  EXPECT_TRUE(controller.IsOn());

  controller.Update(10.0f);
  EXPECT_TRUE(controller.IsOn());
}

TEST(SchmittTriggerRelayTest, TurnsOffAtOrAboveMax) {
  Relay relay(17, false);
  SchmittTriggerRelay controller(relay, 30.0f, 70.0f);

  controller.Update(20.0f);
  ASSERT_TRUE(controller.IsOn());

  controller.Update(70.0f);
  EXPECT_FALSE(controller.IsOn());
}

TEST(SchmittTriggerRelayTest, HoldsStateInsideHysteresisBand) {
  Relay relay(17, false);
  SchmittTriggerRelay controller(relay, 30.0f, 70.0f);

  controller.Update(50.0f);
  EXPECT_FALSE(controller.IsOn());

  controller.Update(20.0f);
  ASSERT_TRUE(controller.IsOn());

  controller.Update(50.0f);
  EXPECT_TRUE(controller.IsOn());
}

TEST(SchmittTriggerRelayTest, RejectsInvertedThresholds) {
  Relay relay(17, false);

  EXPECT_THROW(SchmittTriggerRelay(relay, 70.0f, 30.0f),
               std::invalid_argument);
}
