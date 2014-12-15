//
//  parseerrorlist.cpp
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "parseerrorlist.h"

namespace csoup {
    ParseErrorList::ParseErrorList(size_t maxSize, Allocator* allocator)  :
    maxSize_(maxSize),
    errorList_(maxSize > INITIAL_CAPACITY ? INITIAL_CAPACITY : maxSize, allocator) {
        
    }
}
