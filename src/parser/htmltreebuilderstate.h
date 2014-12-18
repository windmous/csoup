//
//  htmltreebuilderstate.h
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_HTML_TREEBUILDER_STATE_H_
#define CSOUP_HTML_TREEBUILDER_STATE_H_

#include "treebuilder.h"
#include "../util/stringref.h"

namespace csoup {
    class StartTagToken;
    class HtmlTreeBuilder;
    class Token;
    
    class HtmlTreeBuilderState {
    public:
        virtual ~HtmlTreeBuilderState();
        virtual bool process(Token* t, HtmlTreeBuilder* tb);
        
    protected:
        struct TokenDeleter {
            TokenDeleter(Token* t, Allocator* allocator) : token_(t), allocator_(allocator) {}
            ~TokenDeleter();
            
            Token* token_;
            Allocator* allocator_;
        };
        
        static const StringRef nullString_;
        
        static bool isWhitespace(Token* t);
        
        static void handleRcData(StartTagToken* startTag, HtmlTreeBuilder* tb);
        
        static void handleRawtext(StartTagToken* startTag, HtmlTreeBuilder* tb);
        
        class Constants {
        public:
            static const StringRef InBodyStartToHead[];
            static const StringRef InBodyStartPClosers[];
            static const StringRef Headings[];
            static const StringRef InBodyStartPreListing[];
            static const StringRef InBodyStartLiBreakers[];
            static const StringRef DdDt[];
            static const StringRef Formatters[];
            static const StringRef InBodyStartApplets[];
            static const StringRef InBodyStartEmptyFormatters[];
            static const StringRef InBodyStartMedia[];
            static const StringRef InBodyStartInputAttribs[];
            static const StringRef InBodyStartOptions[];
            static const StringRef InBodyStartRuby[];
            static const StringRef InBodyStartDrop[];
            static const StringRef InBodyEndClosers[];
            static const StringRef InBodyEndAdoptionFormatters[];
            static const StringRef InBodyEndTableFosters[];
        };
    };
    
#define CSOUP_REGISTER_HTMLTREEBUILDER_STATE(StateName) \
    class StateName : public HtmlTreeBuilderState { \
    public: \
        bool process(Token* t, HtmlTreeBuilder* tb); \
        static StateName* instance() { \
            static StateName globalInstance; \
            return &globalInstance; \
        }\
    };
    
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(Initial)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(BeforeHtml)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(BeforeHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InHeadNoscript)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(AfterHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(Text)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InTable)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InTableText)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InCaption)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InColumnGroup)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InTableBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InRow)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InCell)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InSelect)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InSelectInTable)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(AfterBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(InFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(AfterFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(AfterAfterBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(AfterAfterFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE(ForeignContent)
    
#undef CSOUP_REGISTER_HTML_TREEBUILDER_STATE
}

#endif // CSOUP_HTML_TREEBUILDER_STATE_H_
