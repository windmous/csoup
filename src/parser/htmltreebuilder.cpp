//
//  htmltreebuilder.cpp
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//
#include "htmltreebuilder.h"
#include "token.h"
#include "util/stringutil.h"
#include "../nodes/element.h"
#include "../nodes/document.h"
#include "../internal/list.h"

namespace csoup {
    const StringRef HtmlTreeBuilder::TagsScriptStyle[]      = {"script", "style"};
    const StringRef HtmlTreeBuilder::TagsSearchInScope[]    = {"applet", "caption", "html", "table", "td", "th", "marquee", "object"};
    const StringRef HtmlTreeBuilder::TagSearchList[]        = {"ol", "ul"};
    const StringRef HtmlTreeBuilder::TagSearchButton[]      = {"button"};
    const StringRef HtmlTreeBuilder::TagSearchTableScope[]  = {"html", "table"};
    const StringRef HtmlTreeBuilder::TagSearchSelectScope[] = {"optgroup", "option"};
    const StringRef HtmlTreeBuilder::TagSearchEndTags[]     = {"dd", "dt", "li", "option", "optgroup", "p", "rp", "rt"};
    const StringRef HtmlTreeBuilder::TagSearchSpecial[]     = {"address", "applet", "area", "article", "aside", "base", "basefont", "bgsound",
        "blockquote", "body", "br", "button", "caption", "center", "col", "colgroup", "command", "dd",
        "details", "dir", "div", "dl", "dt", "embed", "fieldset", "figcaption", "figure", "footer", "form",
        "frame", "frameset", "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hgroup", "hr", "html",
        "iframe", "img", "input", "isindex", "li", "link", "listing", "marquee", "menu", "meta", "nav",
        "noembed", "noframes", "noscript", "object", "ol", "p", "param", "plaintext", "pre", "script",
        "section", "select", "style", "summary", "table", "tbody", "td", "textarea", "tfoot", "th", "thead",
        "title", "tr", "ul", "wbr", "xmp"};
    
    HtmlTreeBuilder::HtmlTreeBuilder(Allocator* allocator) :
    state_(NULL), originalState_(NULL), baseUriSetFromDoc_(false), headElement_(NULL),
    /*formElement(NULL),*/ contextElement_(NULL), formattingElements_(NULL), pendingTableCharacters_(NULL),
    framesetOk_(true), fosterInserts_(false), fragmentParsing_(false), allocator_(allocator) {
        CSOUP_ASSERT(allocator != NULL);
        
        using internal::List;
        
        formattingElements_ = new (allocator->malloc_t< List<Element*> >()) List<Element*>(allocator);
        pendingTableCharacters_ = new (allocator->malloc_t< List<CharacterToken*> >()) List<CharacterToken*>();
    }
    
    HtmlTreeBuilder::~HtmlTreeBuilder() {
        //allocator_->deconstructAndFree(state_);
        //allocator_->deconstructAndFree(originalState_);
        
        //allocator_->deconstructAndFree(headElement_);
        //allocator_->deconstructAndFree(contextElement_);
        allocator_->deconstructAndFree(formattingElements_);
        allocator_->deconstructAndFree(pendingTableCharacters_);
    }
    
    Document* HtmlTreeBuilder::parse(const csoup::StringRef &input, const csoup::StringRef &baseUri, csoup::ParseErrorList *errors, csoup::Allocator *allocator) {
        // reset builder state
        formattingElements_->clear();
        pendingTableCharacters_->clear();
        
        //state_ = ; // set to INITIAL state
        originalState_ = NULL;
        contextElement_ = NULL;
        baseUriSetFromDoc_ = false;
        headElement_ = NULL;
        contextElement_ = NULL;
        framesetOk_ = true;
        fosterInserts_ = false;
        fragmentParsing_ = false;
        
        return TreeBuilder::parse(input, baseUri, errors, allocator);
    }
    
    internal::Vector<Node>* HtmlTreeBuilder::parseFragment(const StringRef& inputFragment, Element* context,
                                                           const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator) {
        // reset builder state
        formattingElements_->clear();
        pendingTableCharacters_->clear();
        
        //state_ = ; // set to INITIAL state
        originalState_ = NULL;
        contextElement_ = context;
        baseUriSetFromDoc_ = false;
        headElement_ = NULL;
        contextElement_ = NULL;
        framesetOk_ = true;
        fosterInserts_ = false;
        fragmentParsing_ = true;
        
        initialiseParse(inputFragment, baseUri, errors, allocator);
        
        Element* root = NULL;
        if (context != NULL) {
            if (context->ownerDocument() != NULL) // quirks setup:
                doc_->setQuirksMode(context->ownerDocument()->quirksMode());
            
            // initialise the tokeniser state:
            StringRef contextTag = context->tagName();
            if (StringUtil::in(contextTag, "title", "textarea"))
                tokeniser_->transition(TokeniserState.Rcdata);
            else if (StringUtil::in(contextTag, "iframe", "noembed", "noframes", "style", "xmp"))
                tokeniser_->transition(TokeniserState.Rawtext);
            else if (contextTag.equals("script"))
                tokeniser_->transition(TokeniserState.ScriptData);
            else if (contextTag.equals(("noscript")))
                tokeniser_->transition(TokeniserState.Data); // if scripting enabled, rawtext
            else if (contextTag.equals("plaintext"))
                tokeniser.transition(TokeniserState.Data);
            else
                tokeniser.transition(TokeniserState.Data); // default
            
            root = new Element(Tag.valueOf("html"), baseUri);
            doc.appendChild(root);
            stack.push(root);
            resetInsertionMode();
            
            // setup form element to nearest form on context (up ancestor chain). ensures form controls are associated
            // with form correctly
            Elements contextChain = context.parents();
            contextChain.add(0, context);
            for (Element parent: contextChain) {
                if (parent instanceof FormElement) {
                    formElement = (FormElement) parent;
                    break;
                }
            }
        }
        
        runParser();
        if (context != null)
            return root.childNodes();
        else
            return doc.childNodes();
    }
    
    
}
