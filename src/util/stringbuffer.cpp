//
//  stringbuffer.cpp
//  csoup
//
//  Created by mac on 12/11/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "stringbuffer.h"

namespace csoup {
    void StringBuffer::ensureExtraSize(size_t extraSize) {
        size_t newLength = length_ + extraSize;
        size_t newCapacity = capacity_;
        
        while (newCapacity < newLength) {
            newCapacity *= 2;
        }
        
        if (newCapacity != capacity_) {
            str_ = allocator_->realloc_t<CharType>(str_, capacity_, newCapacity);
            capacity_ = newCapacity;
        }
    }
    
    void StringBuffer::appendString(const CharType* src, size_t len) {
        ensureExtraSize(len);
        std::memcpy(str_ + length_, src, sizeof(CharType) * len);
        length_ += len;
    }
    
    void StringBuffer::append(int c) {
        int numBytes, prefix;
        if (c <= 0x7f) {
            numBytes = 0;
            prefix = 0;
        } else if (c <= 0x7ff) {
            numBytes = 1;
            prefix = 0xc0;
        } else if (c <= 0xffff) {
            numBytes = 2;
            prefix = 0xe0;
        } else {
            numBytes = 3;
            prefix = 0xf0;
        }
        ensureExtraSize(numBytes + 1);
        str_[length_++] = prefix | (c >> (numBytes * 6));
        for (int i = numBytes - 1; i >= 0; --i) {
            str_[length_++] = 0x80 | (0x3f & (c >> (i * 6)));
        }
    }
    
    void StringBuffer::tolower() {
        for (size_t i = 0; i < length_; ++ i) {
            str_[i] = std::tolower(str_[i]);
        }
    }
    
    void StringBuffer::toupper() {
        for (size_t i = 0; i < length_; ++ i) {
            str_[i] = std::toupper(str_[i]);
        }
    }
}