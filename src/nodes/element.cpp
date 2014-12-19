//
//  element.cpp
//  csoup
//
//  Created by mac on 12/18/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "element.h"
#include "../selector/elementsref.h"

namespace csoup {
    void Element::accumulateParents(csoup::Element *ele, csoup::ElementsRef *output) {
        Node* parNode = ele->parentNode();
        if (parNode == NULL) return ;
        
        
        CSOUP_ASSERT(isElementNode(parNode));
        Element* parElement = (Element*)parNode;
        if (!parElement->tagName().equals("#root")) {
            output->append(parElement);
            accumulateParents(parElement, output);
        }
    }
}