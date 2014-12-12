//
//  stringbuffer.h
//  csoup
//
//  Created by mac on 12/11/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_STRINGBUFFER_H_
#define CSOUP_STRINGBUFFER_H_

#include "common.h"
#include "allocators.h"
#include "stringref.h"
#include "csoup_string.h"

namespace csoup {
    class StringBuffer {
    public:
        typedef Allocator AllocatorType;
        
        StringBuffer(Allocator* allocator)
        : str_(NULL), allocator_(allocator), capacity_(0), length_(0) {
            CSOUP_ASSERT(allocator != NULL);
        }
        
        ~StringBuffer() {
            if (str_ != NULL) {
                allocator_->free(str_);
                str_ = NULL;
                length_ = capacity_ = 0;
            }
        }
        
        void clear() {
            length_ = 0;
        }
        
        const char* data() const {
            return str_ == NULL ? "" : str_;
        }
        
        StringRef ref() const {
            return StringRef(str_, length_);
        }
        
        size_t size() const {
            return length_;
        }
        
        size_t capacity() const {
            return capacity_;
        }
        
        void reserve(size_t expectedSize) {
            if (expectedSize > capacity_) {
                ensureExtraSize(expectedSize - capacity_);
            }
        }
        
        void append(int codePoint);
        
        void appendString(const CharType* src, size_t len);
        
        void appendString(const StringRef& str) {
            appendString(str.data(), str.size());
        }
        
        void appendString(const String& str) {
            appendString(str.data(), str.size());
        }
        
        Allocator* allocator() {
            return allocator_;
        }
    private:
        void ensureExtraSize(size_t extraSize);
        
        CharType* str_;
        Allocator* allocator_;
        size_t capacity_;
        size_t length_;
    };
}

#endif // CSOUP_STRINGBUFFER_H_
