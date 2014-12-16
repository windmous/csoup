//
//  htmltreebuilder.h
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_HTML_TREEBUILDER_H_
#define CSOUP_HTML_TREEBUILDER_H_

#include "treebuilder.h"

namespace csoup {
    class HtmlTreeBuilder : public TreeBuilder {
    public:
        HtmlTreeBuilder();
        
    protected:
        bool process(Token* token);
    };
}

#endif // CSOUP_HTML_TREEBUILDER_H_
