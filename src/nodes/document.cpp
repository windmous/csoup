//
//  document.cpp
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "../util/stringref.h"
#include "../util/csoup_string.h"
#include "token.h"
#include "document.h"

namespace csoup {
    Document::Document(const StringRef& baseUri, Allocator* allocator) :
    Element(CSOUP_NAMESPACE_HTML, CSOUP_TAG_HTML, NULL, allocator ? allocator : new MemoryPoolAllocator()),
    quirksMode_(CSOUP_DOCTYPE_NO_QUIRKS), ownAllocator_(NULL), publicIdentifier_(NULL),
    systemIdentifier_(NULL), name_(NULL), baseUri_(NULL) {
        if (allocator == NULL) {
            ownAllocator_ = Element::allocator();
        }
        
        baseUri_ = new (Node::allocator()->malloc_t<String>()) String(baseUri, Node::allocator());
    }
    
    Document::~Document() {
        allocator()->deconstructAndFree(publicIdentifier_);
        allocator()->deconstructAndFree(systemIdentifier_);
        allocator()->deconstructAndFree(name_);
        
        // it's not necessary to check if ownAllocator_ is NULL or not;
        delete ownAllocator_;
    }
    
    void Document::setSystemIdentifier(const csoup::StringRef &systemIdentifier) {
        CSOUP_DELETE(allocator(), systemIdentifier_);
        systemIdentifier_ = CSOUP_NEW2(allocator(), String, systemIdentifier, allocator());
    }
    
    void Document::setPublicIdentifier(const csoup::StringRef &publicIdentifier) {
        CSOUP_DELETE(allocator(), publicIdentifier_);
        publicIdentifier_ = CSOUP_NEW2(allocator(), String, publicIdentifier, allocator());
    }
    
    void Document::setName(const csoup::StringRef &name) {
        CSOUP_DELETE(allocator(), name_);
        name_ = CSOUP_NEW2(allocator(), String, name, allocator());
    }
}