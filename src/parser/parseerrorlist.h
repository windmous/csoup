//
//  parseerrorlist.h
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_PARSE_ERROR_LIST_H_
#define CSOUP_PARSE_ERROR_LIST_H_

#include "../internal/vector.h"
#include "parseerror.h"

namespace csoup {
    class ParseErrorList {
    public:
        ParseErrorList(size_t maxSize, Allocator* allocator) :
                maxSize_(maxSize),
                errorList_(maxSize > INITIAL_CAPACITY ? INITIAL_CAPACITY : maxSize, allocator) {
            CSOUP_ASSERT(allocator != NULL);
        }
        
        bool notFull() const {
            return errorList_.size() < maxSize_;
        }
        
        size_t maxSize() const {
            return maxSize_;
        }
        
        ParseError* appendError() {
            return notFull() ? errorList_.push() : NULL;
        }
    private:
        static const int INITIAL_CAPACITY = 16;
        
        internal::Vector<ParseError> errorList_;
        size_t maxSize_;
    };
}

#endif // CSOUP_PARSE_ERROR_LIST_H_
