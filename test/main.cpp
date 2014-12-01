//
//  main.cpp
//  test
//
//  Created by mac on 12/1/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include <iostream>
#include "gtest/gtest/gtest.h"

TEST(FooTest, HandleNoneZeroInput)
{
    EXPECT_EQ(2, 2);
    EXPECT_EQ(6, 3);
}
