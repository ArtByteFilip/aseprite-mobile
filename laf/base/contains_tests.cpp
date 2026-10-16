// LAF Base Library
// Copyright (c) 2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gtest/gtest.h>

#include "base/contains.h"

#include <vector>

using namespace base;

TEST(Contains, Base)
{
  EXPECT_FALSE(contains(std::vector<int>{}, 3));
  EXPECT_FALSE(contains(std::vector<int>{ 1, 2 }, 3));
  EXPECT_TRUE(contains(std::vector<int>{ 1, 2, 3 }, 3));
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
