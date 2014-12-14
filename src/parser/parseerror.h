//
//  parseerror.h
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_PARSE_ERROR_H_
#define CSOUP_PARSE_ERROR_H_

#include "../util/common.h"
#include "../util/stringref.h"
#include "../util/csoup_string.h"

namespace csoup {
    class ParseError {
    public:
        ParseError(long pos, const StringRef& errorMsg, Allocator* allocator) :
            pos_(pos),
            errorMsg_(errorMsg, allocator){
            
        }
        
        long pos() const {
            return pos_;
        }
        
        StringRef errorMessage() const {
            return errorMsg_.ref();
        }
    private:
        long pos_;
        String errorMsg_;
    };
}

#endif // CSOUP_PARSE_ERROR_H_
