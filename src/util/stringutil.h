//
//  stringutil.cpp
//  csoup
//
//  Created by mac on 12/16/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_STRING_UTIL_H_
#define CSOUP_STRING_UTIL_H_

#include "common.h"

// This module provides some utility methods for string

namespace csoup {
    class String;
    class StringRef;
    class Allocator;
    class StringBuffer;
    
    class StringUtil {
    public:
        static void join();
        
        static String* padding(size_t width, Allocator* allocator);
        
        static bool isBlank(const StringRef& str);
        
        static bool isBlank(const CharType* str);
        
        static bool isNumeric(const StringRef& str);
        
        static bool isNumeric(const CharType* str);
        
        static bool isWhitespace(int c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r';
        }
        
        static String* normaliseWhitespace(const StringRef* str, Allocator* allocator);
        
        static void appendNormalisedWhitespace(StringBuffer* accum, StringRef& string, bool stripLeading);
        
        static bool in(const StringRef& target, const StringRef& s1);
        static bool in(const StringRef& target, const StringRef& s1, const StringRef& s2);
        static bool in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3);
        static bool in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                       const StringRef& s4);
        static bool in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                       const StringRef& s4, const StringRef& s5);
        static bool in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                       const StringRef& s4, const StringRef& s5, const StringRef& s6);
        
        static bool in(const StringRef& taget, const StringRef* testset, size_t len);
        
    private:
        static const CharType* padding_[];
    };
}

#endif // CSOUP_STRING_UTIL_H_
