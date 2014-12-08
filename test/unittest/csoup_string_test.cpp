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
    //CrtAllocator allocator;
    for (size_t i = 0; i < strs_.size(); ++i) {
        StringRef s(strs_[i]);
        EXPECT_EQ(std::strlen(strs_[i]), s.size())
        << "Failure when test length equality: \"" << strs_[i] << "\"";
        
        EXPECT_STREQ(strs_[i], s.data())
        << "Failure when test string content equality: \"" << strs_[i] << "\"";
        
        EXPECT_EQ(strs_[i], s.data())
        << "Failure when test string pointer equality: \"" << strs_[i] << "\"";
        
        //internal::destroy(&s, &allocator);
    }
}

TEST_F(StringTest, RawdataTestForPtrAndLength)
{
//    CrtAllocator allocator;
    for (size_t i = 0; i < strs_.size(); ++ i) {
        const size_t len = std::strlen(strs_[i]);
        StringRef s(strs_[i], len);

        EXPECT_EQ(len, s.size())
        << "Failure when test length equality: \"" << strs_[i] << "\"";
        
        EXPECT_STREQ(strs_[i], s.data())
        << "Failure when test string content equality: \"" << strs_[i] << "\"";
        
        EXPECT_EQ(strs_[i], s.data())
        << "Failure when test string pointer equality: \"" << strs_[i] << "\"";
        
        //internal::destroy(&s, &allocator);
    }
}


TEST_F(StringTest, RawdataTestForConstString)
{
    //CrtAllocator allocator;
#define DO_TESTCASE_FOR_CONST_STRING(constStr) \
    {                                                   \
        const size_t strLength = std::strlen(constStr); \
        StringRef s(constStr);\
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
        
        StringRef refStr(buffer);
        String copyStr(strs_[i], allocator);
        
        EXPECT_TRUE(internal::strEquals(copyStr,refStr));
        EXPECT_TRUE(internal::strEquals(refStr, copyStr));
        
        EXPECT_TRUE(internal::strEquals(copyStr, copyStr));
        EXPECT_TRUE(internal::strEquals(refStr, refStr));
        
        EXPECT_EQ(refStr.data(), buffer);
        
#define DO_INCASESENSITIVE_STRINGEQUALITY_TEST \
        EXPECT_TRUE(internal::strEqualsIgnoreCase(copyStr, refStr)); \
        EXPECT_TRUE(internal::strEqualsIgnoreCase(refStr, copyStr)); \
        EXPECT_TRUE(internal::strEqualsIgnoreCase(copyStr, copyStr)); \
        EXPECT_TRUE(internal::strEqualsIgnoreCase(refStr, refStr)); \

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
                buffer[i] += (buffer[i] >= 127 ? -1 : 1);
            }
            
            EXPECT_FALSE(internal::strEquals(copyStr, refStr));
            EXPECT_FALSE(internal::strEquals(refStr, copyStr));
            EXPECT_TRUE(internal::strEquals(copyStr, copyStr));
            EXPECT_TRUE(internal::strEquals(refStr, refStr));
        }
        
        internal::destroy(&copyStr, allocator);
        std::free(buffer);
        
        // note that, if copyStr is just an empty string, namely "",
        // then allocator will not do any work.
        EXPECT_TRUE(copyStr.data() == NULL) << "current string is \""
                                                                    << copyStr.data() << "\"";
    }
    
    delete allocator;
}

TEST_F(StringTest, StringInEqualityTest)
{
    for (size_t i = 0; i < strs_.size(); ++ i) {
        StringRef sa(strs_[i]);
        
        for (size_t j = i + 1; j < strs_.size(); ++ j) {
            StringRef sb(strs_[j]);
            EXPECT_FALSE(internal::strEquals(sa, sb));
        }
    }
}