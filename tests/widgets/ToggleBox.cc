#include "gtest/gtest.h"

#include "testutils.h"

#include "widgets/ToggleBox.h"

TEST(ToggleBox, instantiate)
{
  ToggleBox tb("title");
  ToggleBox tb2("title", nullptr);
}

TEST(ToggleBox, initialState)
{
  ToggleBox tb("title");
  EXPECT_TRUE(tb.isCollapsed());
  EXPECT_FALSE(tb.isExpanded());
}

TEST(ToggleBox, expandEmits)
{
  ToggleBox tb("title");

  {
    auto spy = SIGNAL_SPY_ZERO(&tb, &ToggleBox::expanded);
    auto spy2 = SIGNAL_SPY_ONE_FUNC(bool, &tb, &ToggleBox::stateChanged,
                                    [](bool collapsed) { EXPECT_FALSE(collapsed); });
    tb.setExpanded();
  }
}

TEST(ToggleBox, collapseEmits)
{
  ToggleBox tb("title");

  {
    auto spy = SIGNAL_SPY_ZERO(&tb, &ToggleBox::collapsed);
    auto spy2 = SIGNAL_SPY_ONE_FUNC(bool, &tb, &ToggleBox::stateChanged,
                                    [](bool collapsed) { EXPECT_TRUE(collapsed); });
    tb.setCollapsed();
  }
}
