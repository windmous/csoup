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
        virtual ~HtmlTreeBuilderState() = 0;
        virtual bool process(Token* t, HtmlTreeBuilder* tb) = 0;
        
    protected:
        struct TokenDeleter {
            TokenDeleter(Token* t, Allocator* allocator) : token_(t), allocator_(allocator) {}
            ~TokenDeleter();
            
            Token* token_;
            Allocator* allocator_;
        };
        
        bool processExtraEndTagToken(const StringRef& tagName, HtmlTreeBuilder* tb);
        
        bool processExtraStartTagToken(const StringRef& tagName, HtmlTreeBuilder* tb);
        bool processExtraCharToken(const StringRef& data, HtmlTreeBuilder* tb);
        
        bool processExtraToken(Token* token, HtmlTreeBuilder* tb);
        
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
    
    inline HtmlTreeBuilderState::~HtmlTreeBuilderState() {}
    
#define CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(StateName) \
    class StateName : public HtmlTreeBuilderState { \
    public: \
        ~StateName() {}\
        bool process(Token* t, HtmlTreeBuilder* tb); \
        static StateName* instance() { \
            static StateName globalInstance; \
            return &globalInstance; \
        }\
    private: \
    
    
#define CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END };

    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(Initial)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(BeforeHtml)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(BeforeHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InHeadNoscript)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(AfterHead)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(Text)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InTable)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InTableText)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InCaption)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InColumnGroup)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InTableBody)
    bool exitTableBody(Token* t, HtmlTreeBuilder* tb);
    bool anythingElse(Token* t, HtmlTreeBuilder* tb);
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InRow)
    bool anythingElse(Token* t, HtmlTreeBuilder* tb);
    bool handleMissingTr(Token* t, HtmlTreeBuilder* tb);
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InCell)
    bool anythingElse(Token* t, HtmlTreeBuilder* tb);
    void closeCell(HtmlTreeBuilder* tb);
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InSelect)
    bool anythingElse(Token* t, HtmlTreeBuilder* tb);
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InSelectInTable)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(AfterBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(InFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(AfterFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(AfterAfterBody)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(AfterAfterFrameset)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_BEGIN(ForeignContent)
    CSOUP_REGISTER_HTMLTREEBUILDER_STATE_END
    
#undef CSOUP_REGISTER_HTML_TREEBUILDER_STATE
}

#endif // CSOUP_HTML_TREEBUILDER_STATE_H_
