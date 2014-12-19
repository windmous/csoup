//
//  formelement.cpp
//  csoup
//
//  Created by mac on 12/19/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "formelement.h"
#include "../selector/elementsref.h"

namespace csoup {
    void FormElement::appendElementToForm(csoup::Element *ele) {
        ensureElementsRef()->append(ele);
    }
    
    ElementsRef* FormElement::ensureElementsRef() {
        return elements_ ? elements_ : elements_ = (CSOUP_NEW1(allocator(), ElementsRef, allocator()));
    }
    
    FormElement::~FormElement() {
        CSOUP_DELETE(allocator(), elements_);
    }
}