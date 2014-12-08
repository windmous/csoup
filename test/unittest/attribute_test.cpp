#include <iostream>
#include <vector>
#include <cstring>
#include <cctype>
#include "gtest/gtest/gtest.h"
#include "nodes/attribute.h"
#include "util/allocators.h"

using namespace csoup;

//TEST(AttributeTest, SimpleTest)
//{
//    StringRef k1("k1");
//    StringRef v1("v1");
//    StringRef v2("v2");
//    
//    CrtAllocator allocator;
//    
//    
//    Attribute attr1(k1, v1);
//    Attribute attr2(k1.data(), v1.data(), &allocator);
//    
//    EXPECT_TRUE(attr1 == attr2);
//    EXPECT_TRUE(attr1.key() == k1);
//    EXPECT_TRUE(attr2.value() == v1);
//
//    String k2("k2", &allocator);
//    attr1.setKey(k2.data(), &allocator);
//
//    EXPECT_FALSE(attr1 == attr2);
//    attr2.setKey(k2, &allocator);
//    EXPECT_TRUE(attr2 == attr1);
//    
//    internal::destroy(&attr1, &allocator);
//    std::cout << "error1" << std::endl;
//    internal::destroy(&attr2, &allocator);
//    std::cout << "error2" << std::endl;
//    internal::destroy(&k2, &allocator);
//    std::cout << "error3" << std::endl;
//    internal::destroy(&v2, &allocator);
//    std::cout << "error4" << std::endl;
//}