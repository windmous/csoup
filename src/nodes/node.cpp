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
    
    Element* Node::parentNode() {
        if (!parent_) return NULL;
        CSOUP_ASSERT(Element::isElementNode(parent_));
        return (Element*)parent_;
    }
    
    void Node::removeFromParent(bool del) {
        Element* parent = parentNode();
        parent->removeChild(this, del);
    }
    
    void Node::setParentNode(csoup::Node *parent) {
        CSOUP_ASSERT(parent == NULL || Element::isElementNode(parent));
        CSOUP_ASSERT(parentNode() == NULL);
        parent_ = parent;
    }
    
    void Node::after(csoup::Node *node) {
        parentNode()->insertNode(siblingIndex(), node);
    }
    
    void Node::before(csoup::Node *node) {
        parentNode()->insertNode(siblingIndex() + 1, node);
    }
}