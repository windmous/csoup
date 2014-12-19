//
//  elements.cpp
//  csoup
//
//  Created by mac on 12/19/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "elementsref.h"
#include "../nodes/element.h"

namespace csoup {
    StringRef ElementsRef::attr(const csoup::StringRef &key) const {
        for (size_t i = 0; i < contents_.size(); ++ i) {
            Element* ele = *contents_.at(i);
            if (ele->hasAttribute(key)) {
                return ele->attr(key);
            }
        }
        
        return StringRef("");
    }
    
    bool ElementsRef::hasAttribute(const csoup::StringRef &key) const {
        for (size_t i = 0; i < contents_.size(); ++ i) {
            Element* ele = *contents_.at(i);
            if (ele->hasAttribute(key)) {
                return true;
            }
        }
        
        return false;
    }
    
    
}