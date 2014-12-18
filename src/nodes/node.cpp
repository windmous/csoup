//
//  node.cpp
//  csoup
//
//  Created by mac on 12/17/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "node.h"
#include "element.h"
#include "document.h"

namespace csoup {
    Document* Node::ownerDocument() const {
        if (type() == CSOUP_NODE_DOCUMENT) {
            // I don't use dynamic_cast here because we have checked.
            return (Document*)(this);
        } else if (parentNode() == NULL) {
            return NULL;
        } else {
            return parentNode()->ownerDocument();
        }
    }
}