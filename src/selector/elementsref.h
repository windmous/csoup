//
//  elements.h
//  csoup
//
//  Created by mac on 12/19/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#define CSOUP_TOKEN_H_
#define CSOUP_TOKEN_H_

#include "../internal/vector.h"

namespace csoup {
    class Element;
    
    class ElementsRef {
    public:
        Elements(Allocator* allocator) : contents_(1, allocator) {
            
        }
        
        Elements(size_t initialCapacity, Allocator* allocator) : contents_(initialCapacity, allocator) {
            
        }
        
        
        
    private:
        internal::Vector<Element*> contents_;
    };
}

#endif // CSOUP_TOKEN_H_
