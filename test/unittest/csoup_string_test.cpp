//
//  csoup_string_test.cpp
//  test
//
//  Created by mac on 12/7/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

// TODO:
//      1. Add testcases for deepcopy
//      2. Add testcases for case-insentitive comparing

#include <iostream>
#include <vector>
#include <cstring>
#include <cctype>
#include "gtest/gtest/gtest.h"
#include "util/csoup_string.h"
#include "util/allocators.h"

using namespace csoup;

class StringTest : public testing::Test {
protected:
    // we add some strings to be tested
    static void SetUpTestCase() {
        const char* cases[] = {
            "",             // empty string
            "B",
            ",",
            "a",            // string is one
            "hello world",   // a longer string
            "Btrjo jtro If", //
            "你",
            "C++ is Hard!", //
            "微软：Microsoft"
        };
        
        const size_t case_count = sizeof(cases) / sizeof(*cases);
        for (size_t i = 0; i < case_count; ++ i)
            strs_.push_back(cases[i]);
    }
    
    static void TearDownTestCase() {
        
    }
    
    static std::vector<const char*> strs_;
    //static const char emptyConstStr[];// = "";
    //static const char oneCharConstStr[];// = "a";
    //static const char shortConstStr[];// = "hello world";
};

std::vector<const char*> StringTest::strs_;

TEST_F(StringTest, RawdataTestForPtrOnly)
{
    for (size_t i = 0; i < strs_.size(); ++i) {
        String s = String::fromRawData(strs_[i]);
        EXPECT_EQ(std::strlen(strs_[i]), s.size())
        << "Failure when test length equality: \"" << strs_[i] << "\"";
        
        EXPECT_STREQ(strs_[i], s.data())
        << "Failure when test string content equality: \"" << strs_[i] << "\"";
        
        EXPECT_EQ(strs_[i], s.data())
        << "Failure when test string pointer equality: \"" << strs_[i] << "\"";
    }
}

TEST_F(StringTest, RawdataTestForPtrAndLength)
{
    for (size_t i = 0; i < strs_.size(); ++ i) {
        const size_t len = std::strlen(strs_[i]);
        String s = String::fromRawData(strs_[i], len);

        EXPECT_EQ(len, s.size())
        << "Failure when test length equality: \"" << strs_[i] << "\"";
        
        EXPECT_STREQ(strs_[i], s.data())
        << "Failure when test string content equality: \"" << strs_[i] << "\"";
        
        EXPECT_EQ(strs_[i], s.data())
        << "Failure when test string pointer equality: \"" << strs_[i] << "\"";
    }
}


TEST_F(StringTest, RawdataTestForConstString)
{
#define DO_TESTCASE_FOR_CONST_STRING(constStr) \
    {                                                   \
        const size_t strLength = std::strlen(constStr); \
        String s = String::fromRawData(constStr);\
        EXPECT_EQ(strLength, s.size())                      \
            << "Failure when test length equality: \"" << constStr << "\""; \
        EXPECT_STREQ(constStr, s.data())                    \
            << "Failure when test string content equality: \"" << constStr << "\""; \
        EXPECT_EQ(static_cast<const char*>(constStr), s.data())                    \
            << "Failure when test string pointer equality: \"" << constStr << "\""; \
    }
    
    DO_TESTCASE_FOR_CONST_STRING("");
    DO_TESTCASE_FOR_CONST_STRING("a");
    DO_TESTCASE_FOR_CONST_STRING("hello world!");
    
#undef DO_TESTCASE_FOR_CONST_STRING
}

TEST_F(StringTest, StringEqualityTest)
{
    CrtAllocator* allocator = new CrtAllocator();
    for (size_t i = 0; i < strs_.size(); ++ i) {
        char* buffer = static_cast<char*>(std::malloc(128));
        std::strcpy(buffer, strs_[i]);
        
        String refStr = String::fromRawData(buffer);
        String copyStr(strs_[i], allocator);
        
        EXPECT_TRUE(copyStr == refStr);
        EXPECT_TRUE(refStr == copyStr);
        
        EXPECT_TRUE(copyStr == copyStr);
        EXPECT_TRUE(refStr ==  refStr);
        
        EXPECT_EQ(refStr.data(), buffer);
        
#define DO_INCASESENSITIVE_STRINGEQUALITY_TEST \
        EXPECT_TRUE(copyStr.equalsIgnoreCase(refStr)); \
        EXPECT_TRUE(refStr.equalsIgnoreCase(copyStr)); \
        EXPECT_TRUE(copyStr.equalsIgnoreCase(copyStr)); \
        EXPECT_TRUE(refStr.equalsIgnoreCase(copyStr)); \

        DO_INCASESENSITIVE_STRINGEQUALITY_TEST;
        
        for (size_t i = 0; buffer[i] != 0; ++ i) {
            if (i & 1) {
                buffer[i] = std::toupper(buffer[i]);
            } else {
                buffer[i] = std::tolower(buffer[i]);
            }
        }
        
        DO_INCASESENSITIVE_STRINGEQUALITY_TEST;
#undef DO_INCASESENSITIVE_STRINGEQUALITY_TEST
        
        if (refStr.size() > 0) {
            for (size_t i = 0; i < refStr.size(); ++ i) {
                buffer[i] += (buffcd er[i] >= 127 ? -1 : 1);
            }
            
            EXPECT_FALSE(copyStr == refStr);
            EXPECT_FALSE(refStr == copyStr);
            EXPECT_TRUE(copyStr == copyStr);
            EXPECT_TRUE(refStr == refStr);
        }
        
        internal::destroy(&copyStr, allocator);
        std::free(buffer);
    }
}
