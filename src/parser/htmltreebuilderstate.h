//
//  htmltreebuilderstate.h
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_HTML_TREEBUILDER_STATE_H_
#define CSOUP_HTML_TREEBUILDER_STATE_H_

namespace csoup {
    class HtmlTreeBuilder;
    class Token;
    
    class HtmlTreeBuilderState {
    public:
        virtual ~HtmlTreeBuilderState();
        
        virtual bool process(Token* t, HtmlTreeBuilder* tb) = 0;
    };
    
    
}

#endif // CSOUP_HTML_TREEBUILDER_STATE_H_
