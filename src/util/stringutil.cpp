//
//  stringutil.cpp
//  csoup
//
//  Created by mac on 12/16/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "stringutil.h"
#include "csoup_string.h"
#include "stringref.h"
#include "stringbuffer.h"
#include "allocators.h"
#include "internal/strfunc.h"

namespace csoup {
    const CharType* StringUtil::padding_[] = {
        "", " ", "  ", "   ", "    ", "     ", "      ", "       ", "        ", "         ", "          "
    };
    
    bool StringUtil::in(const StringRef& target, const StringRef& s1) {
        return internal::strEquals(target, s1);
    }
    
    bool StringUtil::in(const StringRef& target, const StringRef& s1, const StringRef& s2) {
        return internal::strEquals(target, s1) || internal::strEquals(target, s2);
    }
    
    bool StringUtil::in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3) {
        return internal::strEquals(target, s1) ||
               internal::strEquals(target, s2) ||
               internal::strEquals(target, s3);
    }
    
    bool StringUtil::in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                        const StringRef& s4) {
        return internal::strEquals(target, s1) ||
               internal::strEquals(target, s2) ||
               internal::strEquals(target, s3) ||
               internal::strEquals(target, s4);
    }
    bool StringUtil::in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                        const StringRef& s4, const StringRef& s5) {
        return internal::strEquals(target, s1) ||
               internal::strEquals(target, s2) ||
               internal::strEquals(target, s3) ||
               internal::strEquals(target, s4) ||
               internal::strEquals(target, s5);
    }
    bool StringUtil::in(const StringRef& target, const StringRef& s1, const StringRef& s2, const StringRef& s3,
                        const StringRef& s4, const StringRef& s5, const StringRef& s6) {
        return internal::strEquals(target, s1) ||
               internal::strEquals(target, s2) ||
               internal::strEquals(target, s3) ||
               internal::strEquals(target, s4) ||
               internal::strEquals(target, s5) ||
               internal::strEquals(target, s6);
    }
    
    bool StringUtil::in(const StringRef& target, const StringRef* testset, size_t len) {
        for (size_t i = 0; i < len; ++ i) {
            if (internal::strEquals(target, testset[i])) {
                return true;
            }
        }
        
        return false;
    }
}