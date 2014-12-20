//
//  htmltreebuilderstate.cpp
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "htmltreebuilderstate.h"
#include "htmltreebuilder.h"
#include "token.h"
#include "stringutil.h"
#include "tokeniserstate.h"
#include "document.h"
#include "tokeniser.h"
#include "../nodes/formelement.h"
#include "../internal/list.h"

namespace csoup {
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartToHead[]
            = {"base", "basefont", "bgsound", "command", "link", "meta","noframes", "script", "style", "title"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartPClosers[]
            = {"address", "article", "aside", "blockquote", "center", "details", "dir", "div", "dl","fieldset",
                "figcaption", "figure", "footer", "header", "hgroup", "menu", "nav", "ol","p", "section",
                "summary", "ul"};
    const StringRef HtmlTreeBuilderState::Constants::Headings[]
            = {"h1", "h2", "h3", "h4", "h5", "h6"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartPreListing[]
            = {"pre", "listing"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartLiBreakers[]
            = {"address", "div", "p"};
    const StringRef HtmlTreeBuilderState::Constants::DdDt[]
            = {"dd", "dt"};
    const StringRef HtmlTreeBuilderState::Constants::Formatters[]
            = {"b", "big", "code", "em", "font", "i", "s", "small", "strike", "strong", "tt", "u"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartApplets[]
            = {"applet", "marquee", "object"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartEmptyFormatters[]
            = {"area", "br", "embed", "img", "keygen", "wbr"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartMedia[]
            = {"param", "source", "track"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartInputAttribs[]
            = {"name", "action", "prompt"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartOptions[]
            = {"optgroup", "option"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartRuby[]
            = {"rp", "rt"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyStartDrop[]
            = {"caption", "col", "colgroup", "frame", "head", "tbody", "td", "tfoot", "th", "thead", "tr"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyEndClosers[]
            = {"address", "article", "aside", "blockquote", "button", "center", "details", "dir", "div",
                "dl", "fieldset", "figcaption", "figure", "footer", "header", "hgroup", "listing", "menu",
                "nav", "ol", "pre", "section", "summary", "ul"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyEndAdoptionFormatters[]
            = {"a", "b", "big", "code", "em", "font", "i", "nobr", "s", "small", "strike", "strong", "tt", "u"};
    const StringRef HtmlTreeBuilderState::Constants::InBodyEndTableFosters[]
            = {"table", "tbody", "tfoot", "thead", "tr"};
    
    HtmlTreeBuilderState::TokenDeleter::~TokenDeleter() {
        CSOUP_DELETE(allocator_, token_);
    }
    
    StringRef const HtmlTreeBuilderState::nullString_ = "\x00";
    
    bool HtmlTreeBuilderState::isWhitespace(csoup::Token *t) {
        if (t->tokenType() == CSOUP_TOKEN_CHARACTER) {
            StringRef data = ((CharacterToken*)t)->data();
            for (size_t i = 0; i < data.size(); ++ i) {
                if (!StringUtil::isWhitespace(data.at(i))) {
                    // In UTF-8, this should be only 1 byte;
                    
                    CSOUP_ASSERT(data.size() == 1);
                    return false;
                }
            }
            
            return true;
        }
        return false;
    }
    
    void HtmlTreeBuilderState::handleRawtext(StartTagToken *startTag, HtmlTreeBuilder *tb) {
        tb->insert(startTag);
    }
    
    void HtmlTreeBuilderState::handleRcData(csoup::StartTagToken *startTag, csoup::HtmlTreeBuilder *tb) {
        
    }
    
    bool HtmlTreeBuilderState::processExtraToken(csoup::Token *token, csoup::HtmlTreeBuilder *tb) {
        bool ret = tb->process(token);
        CSOUP_DELETE(tb->allocator(), token);
        return ret;
    }
    
    bool HtmlTreeBuilderState::processExtraEndTagToken(const StringRef &tagName, HtmlTreeBuilder *tb) {
        EndTagToken* endToken = CSOUP_NEW2(tb->allocator(), EndTagToken, tagName, tb->allocator());
        bool ret = tb->process(endToken);
        CSOUP_DELETE(tb->allocator(), endToken);
        
        return ret;
    }
    
    bool HtmlTreeBuilderState::processExtraStartTagToken(const StringRef &tagName, HtmlTreeBuilder *tb) {
        StartTagToken* startToken = CSOUP_NEW2(tb->allocator(), StartTagToken, tagName, tb->allocator());
        bool ret = tb->process(startToken);
        CSOUP_DELETE(tb->allocator(), startToken);
        
        return ret;
    }
    
    bool HtmlTreeBuilderState::processExtraCharToken(const StringRef& data, HtmlTreeBuilder* tb) {
        CharacterToken* charToken = CSOUP_NEW2(tb->allocator(), CharacterToken, data, tb->allocator());
        bool ret = tb->process(charToken);
        CSOUP_DELETE(tb->allocator(), charToken);
        
        return ret;
    }
    
    bool Initial::process(Token* t, HtmlTreeBuilder* tb) {
        //TokenDeleter tokenDeleter(t, tb->allocator());
        
        if (isWhitespace(t)) {
            return true; // ignore whitespace
        } else if (t->isCommentToken()) {
            tb->insert(t->asCommentToken());
        } else if (t->isDoctypeToken()) {
            // todo: parse error check on expected doctypes
            // todo: quirk state check on doctype ids
            DoctypeToken* d = t->asDoctypeToken();
            
            Document* doc = tb->document();
            doc->setName(d->name());
            doc->setPublicIdentifier(d->publicIdentifier());
            doc->setSystemIdentifier(d->systemIdentifier());
            if (d->forceQuirks())
                doc->setQuirksMode(CSOUP_DOCTYPE_QUIRKS);
            tb->transition(BeforeHtml::instance());
        } else {
            // todo: check not iframe srcdoc
            tb->transition(BeforeHtml::instance());
            return tb->process(t); // re-process token
        }
        return true;
    }
    
    bool BeforeHtml::process(Token* t, HtmlTreeBuilder* tb) {
        //TokenDeleter tokenDeleter(t, tb->allocator());
        
        if (t->isDoctypeToken()) {
            tb->error(this);
            return false;
        } else if (t->isCommentToken()) {
            tb->insert(t->asCommentToken());
        } else if (isWhitespace(t)) {
            return true; // ignore whitespace
        } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html")) {
            tb->insert(t->asStartTagToken());
            tb->transition(BeforeHead::instance());
        } else if (t->isEndTagToken() && (StringUtil::in(t->asEndTagToken()->tagName(), "head", "body", "html", "br"))) {
            tb->insert("html");
            tb->transition(BeforeHead::instance());
            return tb->process(t);
        } else if (t->isEndTagToken()) {
            tb->error(this);
            return false;
        } else {
            tb->insert("html");
            tb->transition(BeforeHead::instance());
            return tb->process(t);
        }
        return true;
    }

    
    bool BeforeHead::process(Token* t, HtmlTreeBuilder* tb) {
        //TokenDeleter tokenDeleter(t, tb->allocator());
        
        if (isWhitespace(t)) {
            return true;
        } else if (t->isCommentToken()) {
            tb->insert(t->asCommentToken());
        } else if (t->isDoctypeToken()) {
            tb->error(this);
            return false;
        } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html")) {
            return InBody::instance()->process(t, tb); // does not transition
        } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("head")) {
            Element* head = tb->insert(t->asStartTagToken());
            tb->setHeadElement(head, false);
            tb->transition(InHead::instance());
        } else if (t->isEndTagToken() && (StringUtil::in(t->asEndTagToken()->tagName(), "head", "body", "html", "br"))) {
            processExtraStartTagToken("head", tb);
            return tb->process(t);
        } else if (t->isEndTagToken()) {
            tb->error(this);
            return false;
        } else {
            processExtraStartTagToken("head", tb);
            return tb->process(t);
        }
        return true;
    }
    
#define INHEAD_STATE_ANYTHINGELSE \
    do { \
        processExtraStartTagToken("head", tb); \
        return tb->process(t); \
    } while(false)

    bool InHead::process(Token* t, HtmlTreeBuilder* tb) {
       //TokenDeleter tokenDeleter(t, tb->allocator());
       
       if (isWhitespace(t)) {
           tb->insert(t->asCharacterToken());
           return true;
       }
       switch (t->tokenType()) {
           case CSOUP_TOKEN_COMMENT:
               tb->insert(t->asCommentToken());
               break;
           case CSOUP_TOKEN_DOCTYPE:
               tb->error(this);
               return false;
           case CSOUP_TOKEN_START_TAG: {
               StartTagToken* start = t->asStartTagToken();
               StringRef name = start->tagName();
               if (name.equals("html")) {
                   return InBody::instance()->process(t, tb);
               } else if (StringUtil::in(name, "base", "basefont", "bgsound", "command", "link")) {
                   Element* el = tb->insertEmpty(start);
                   // jsoup special: update base the frist time it is seen
                   if (name.equals("base") && el->hasAttribute("href"))
                       tb->maybeSetBaseUri(el);
               } else if (name.equals("meta")) {
                   //Element* meta = tb->insertEmpty(start);
                   // todo: charset switches
               } else if (name.equals("title")) {
                   handleRcData(start, tb);
               } else if (StringUtil::in(name, "noframes", "style")) {
                   handleRawtext(start, tb);
               } else if (name.equals("noscript")) {
                   // else if noscript && scripting flag = true: rawtext (jsoup doesn't run script, to handle as noscript)
                   tb->insert(start);
                   tb->transition(InHeadNoscript::instance());
               } else if (name.equals("script")) {
                   // skips some script rules as won't execute them
                   
                   tb->setTokeniserState(internal::ScriptData::instance());
                   tb->markInsertionMode();
                   tb->transition(Text::instance());
                   tb->insert(start);
               } else if (name.equals("head")) {
                   tb->error(this);
                   return false;
               } else {
                   INHEAD_STATE_ANYTHINGELSE;
               }
               break;
           }
           case CSOUP_TOKEN_END_TAG: {
               EndTagToken* end = t->asEndTagToken();
               StringRef name = end->tagName();
               if (name.equals("head")) {
                   tb->pop();
                   tb->transition(AfterHead::instance());
               } else if (StringUtil::in(name, "body", "html", "br")) {
                   INHEAD_STATE_ANYTHINGELSE;
               } else {
                   tb->error(this);
                   return false;
               }
               break;
           }
           default: {
               INHEAD_STATE_ANYTHINGELSE;
           }
       }
       return true;
    }
        
#undef INHEAD_STATE_ANYTHINGELSE

#define IHEADNOSCRIPT_ANYTHINGELSE \
do { \
    tb->error(this); \
    processExtraEndTagToken("noscript", tb); \
    return tb->process(t); \
} while(false)

    bool InHeadNoscript::process(Token* t, HtmlTreeBuilder* tb) {
       //TokenDeleter tokenDeleter(t, tb->allocator());
       
       if (t->isDoctypeToken()) {
           tb->error(this);
       } else if (t->isStartTagToken() && internal::strEquals(t->asStartTagToken()->tagName(),"html")) {
           return tb->process(t, InBody::instance());
       } else if (t->isEndTagToken() && internal::strEquals(t->asEndTagToken()->tagName(),"noscript")) {
           tb->pop();
           tb->transition(InHead::instance());
       } else if (isWhitespace(t) || t->asCommentToken() ||
                  (t->isStartTagToken() && StringUtil::in(t->asStartTagToken()->tagName(),
                                                            "basefont", "bgsound", "link", "meta", "noframes", "style"))) {
           return tb->process(t, InHead::instance());
       } else if (t->isEndTagToken() && internal::strEquals(t->asEndTagToken()->tagName(),"br")) {
           IHEADNOSCRIPT_ANYTHINGELSE;
       } else if ((t->isStartTagToken() && StringUtil::in(t->asStartTagToken()->tagName(), "head", "noscript")) || t->isEndTagToken()) {
           tb->error(this);
           return false;
       } else {
           IHEADNOSCRIPT_ANYTHINGELSE;
       }
       return true;
    }
    
#undef IHEADNOSCRIPT_ANYTHINGELSE

    bool AfterHead::process(Token* t, HtmlTreeBuilder* tb) {
       //TokenDeleter tokenDeleter(t, tb->allocator());
       
       if (isWhitespace(t)) {
           tb->insert(t->asCharacterToken());
       } else if (t->isCommentToken()) {
           tb->insert(t->asCommentToken());
       } else if (t->isDoctypeToken()) {
           tb->error(this);
       } else if (t->isStartTagToken()) {
           StartTagToken* startTag = t->asStartTagToken();
           StringRef name = startTag->tagName();
           static const StringRef tagsInHead[] = {"base", "basefont", "bgsound", "link", "meta", "noframes", "script", "style", "title"};
           
           if (name.equals(StringRef("html"))) {
               return tb->process(t, InBody::instance());
           } else if (name.equals("body")) {
               tb->insert(startTag);
               tb->setFramesetOk(false);
               tb->transition(InBody::instance());
           } else if (name.equals("frameset")) {
               tb->insert(startTag);
               tb->transition(InFrameset::instance());
           } else if (StringUtil::in(name, tagsInHead, arrayLength(tagsInHead))) {
               tb->error(this);
               
               // temporarily add it to top of the stack and process, then remove it
               Element* head = tb->headElement();
               tb->push(head);
               tb->process(t, InHead::instance());
               tb->removeFromStack(head, false);
           } else if (name.equals("head")) {
               tb->error(this);
               return false;
           } else {
               processExtraStartTagToken("body", tb);
               tb->setFramesetOk(true);
               return tb->process(t);
           }
       } else if (t->isEndTagToken()) {
           if (StringUtil::in(t->asEndTagToken()->tagName(), "body", "html")) {
               processExtraStartTagToken("body", tb);
               tb->setFramesetOk(true);
               return tb->process(t);
           } else {
               tb->error(this);
               return false;
           }
       } else {
           processExtraStartTagToken("body", tb);

           tb->setFramesetOk(true);
           return tb->process(t);
       }
       return true;
    }
    
    bool InBodyAnyOtherEndTag(HtmlTreeBuilderState* state, Token* t, HtmlTreeBuilder* tb) {
        StringRef name = t->asEndTagToken()->tagName();
        internal::Vector<Element*>* stack = tb->stack();
        internal::VectorIterator<Element*> it = stack->end();
        while (it.hasPrevious()) {
            it.previous();
            
            Element* node = *it.data();
            if (node->tagName().equals(name)) {
                tb->generateImpliedEndTags(name);
                if (!name.equals(tb->currentElement()->tagName()))
                    tb->error(state);
                tb->popStackToClose(false, name);
                break;
            } else {
                if (tb->isSpecial(node)) {
                    tb->error(state);
                    return false;
                }
            }
        }
        return true;
    }

    bool InBody::process(Token* t, HtmlTreeBuilder* tb) {
       //TokenDeleter tokenDeleter(t, tb->allocator());
       
       switch (t->tokenType()) {
           case CSOUP_TOKEN_CHARACTER: {
               CharacterToken* c = t->asCharacterToken();
               if (c->data().equals(nullString_)) {
                   // todo confirm that check
                   tb->error(this);
                   return false;
               } else if (tb->framesetOk() && isWhitespace(c)) { // don't check if whitespace if frames already closed
                   tb->reconstructFormattingElements(false);
                   tb->insert(c);
               } else {
                   tb->reconstructFormattingElements(false);
                   tb->insert(c);
                   tb->setFramesetOk(false);
               }
               break;
           }
           case CSOUP_TOKEN_COMMENT: {
               tb->insert(t->asCommentToken());
               break;
           }
           case CSOUP_TOKEN_DOCTYPE: {
               tb->error(this);
               return false;
           }
           case CSOUP_TOKEN_START_TAG: {
               StartTagToken* startTag = t->asStartTagToken();
               StringRef name = startTag->tagName();
               if (name.equals("html")) {
                   tb->error(this);
                   // merge attributes onto real html
                   Element* html = *tb->stack()->front();
                   Attributes* attrsOfStartTag = startTag->attributes();
                   for (size_t i = 0; i < attrsOfStartTag->size(); ++ i) {
                       const Attribute* attr = attrsOfStartTag->get(i);
                       if (!html->hasAttribute(attr->key())) {
                           html->addAttribute(attr->key(), attr->value());
                       }
                   }
               } else if (StringUtil::in(name, Constants::InBodyStartToHead, arrayLength(Constants::InBodyStartToHead))) {
                   return tb->process(t, InHead::instance());
               } else if (name.equals("body")) {
                   tb->error(this);
                   internal::Vector<Element*>* stack = tb->stack();
                   if (stack->size() == 1 || (stack->size() > 2 && !(*stack->at(1))->tagName().equals("body"))) {
                       // only in fragment case
                       return false; // ignore
                   } else {
                       tb->setFramesetOk(false);
                       Element* body = *stack->at(1);
                       
                       Attributes* attrsOfStartTag = startTag->attributes();
                       for (size_t i = 0; i < attrsOfStartTag->size(); ++ i) {
                           const Attribute* attr = attrsOfStartTag->get(i);
                           if (!body->hasAttribute(attr->key())) {
                               body->addAttribute(attr->key(), attr->value());
                           }
                       }
                       
//                       for (Attribute attribute : startTag.getAttributes()) {
//                           if (!body.hasAttribute(attribute.getKey()))
//                               body.attributes().put(attribute);
//                       }
                   }
               } else if (name.equals("frameset")) {
                   tb->error(this);
                   internal::Vector<Element*>* stack = tb->stack();
                   if (stack->size() == 1 || (stack->size() > 2 && !(*stack->at(1))->tagName().equals("body"))) {
                       // only in fragment case
                       return false; // ignore
                   } else if (!tb->framesetOk()) {
                       return false; // ignore frameset
                   } else {
                       Element* second = *stack->at(1);
                       if (second->parentNode() != NULL) {
                           second->removeFromParent(true);
                       }
                       // pop up to html element
                       while (stack->size() > 1)
                           stack->pop();
                       tb->insert(startTag);
                       tb->transition(InFrameset::instance());
                   }
               } else if (StringUtil::in(name, Constants::InBodyStartPClosers, arrayLength(Constants::InBodyEndAdoptionFormatters))) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
               } else if (StringUtil::in(name, Constants::Headings, arrayLength(Constants::Headings))) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   if (StringUtil::in(tb->currentElement()->tagName(), Constants::Headings, arrayLength(Constants::Headings))) {
                       tb->error(this);
                       tb->pop();
                   }
                   tb->insert(startTag);
               } else if (StringUtil::in(name, Constants::InBodyStartPreListing, arrayLength(Constants::InBodyStartPreListing))) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
                   // todo: ignore LF if next token
                   tb->setFramesetOk(false);
               } else if (name.equals("form")) {
                   if (tb->formElement() != NULL) {
                       tb->error(this);
                       return false;
                   }
                   
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   
                   tb->insert(startTag);
                   tb->insertForm(startTag, true);
               } else if (name.equals("li")) {
                   tb->setFramesetOk(false);
                   internal::Vector<Element*>* stack = tb->stack();
                   for (size_t i = stack->size(); i > 1; i--) {
                       Element* el = *stack->at(i - 1);
                       if (el->tagName().equals("li")) {
                           processExtraEndTagToken("li", tb);
                           break;
                       }
                       if (tb->isSpecial(el) && !StringUtil::in(el->tagName(), Constants::InBodyStartLiBreakers,
                                                                arrayLength(Constants::InBodyStartLiBreakers)))
                           break;
                   }
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
               } else if (StringUtil::in(name, Constants::DdDt, arrayLength(Constants::DdDt))) {
                   tb->setFramesetOk(false);
                   internal::Vector<Element*>* stack = tb->stack();
                   for (size_t i = stack->size(); i > 1; i--) {
                       Element* el = *stack->at(i - 1);
                       if (StringUtil::in(el->tagName(), Constants::DdDt, arrayLength(Constants::DdDt))) {
                           processExtraEndTagToken(el->tagName(), tb);
                           
                           break;
                       }
                       if (tb->isSpecial(el) && !StringUtil::in(el->tagName(), Constants::InBodyStartLiBreakers,
                                                                arrayLength(Constants::InBodyStartLiBreakers)))
                           break;
                   }
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
               } else if (name.equals("plaintext")) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
                   tb->setTokeniserState(internal::PlainText::instance()); // once in, never gets out
               } else if (name.equals("button")) {
                   if (tb->inButtonScope("button")) {
                       // close and reprocess
                       tb->error(this);
                       processExtraEndTagToken("button", tb);
                       
                       tb->process(startTag);
                   } else {
                       tb->reconstructFormattingElements(false);
                       tb->insert(startTag);
                       tb->setFramesetOk(false);
                   }
               } else if (name.equals("a")) {
                   if (tb->getActiveFormattingElement("a") != NULL) {
                       tb->error(this);
                       
                       processExtraEndTagToken("a", tb);
                       
                       // still on stack?
                       Element* remainingA = tb->getFromStack("a");
                       if (remainingA != NULL) {
                           tb->removeFromActiveFormattingElements(remainingA, false);
                           tb->removeFromStack(remainingA, false);
                       }
                   }
                   tb->reconstructFormattingElements(false);
                   Element* a = tb->insert(startTag);
                   tb->pushActiveFormattingElements(a, false);
               } else if (StringUtil::in(name, Constants::Formatters, arrayLength(Constants::Formatters))) {
                   tb->reconstructFormattingElements(false);
                   Element* el = tb->insert(startTag);
                   tb->pushActiveFormattingElements(el, false);
               } else if (name.equals("nobr")) {
                   tb->reconstructFormattingElements(false);
                   if (tb->inScope("nobr")) {
                       tb->error(this);
                       
                       processExtraEndTagToken("nobr", tb);
                       
                       tb->reconstructFormattingElements(false);
                   }
                   Element* el = tb->insert(startTag);
                   tb->pushActiveFormattingElements(el, false);
               } else if (StringUtil::in(name, Constants::InBodyStartApplets,arrayLength(Constants::InBodyStartApplets))) {
                   tb->reconstructFormattingElements(false);
                   tb->insert(startTag);
                   tb->insertMarkerToFormattingElements();
                   tb->setFramesetOk(false);
               } else if (name.equals("table")) {
                   if (tb->document()->quirksMode() != CSOUP_DOCTYPE_QUIRKS && tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insert(startTag);
                   tb->setFramesetOk(false);
                   tb->transition(InTable::instance());
               } else if (StringUtil::in(name, Constants::InBodyStartEmptyFormatters,
                                         arrayLength(Constants::InBodyStartEmptyFormatters))) {
                   tb->reconstructFormattingElements(false);
                   tb->insertEmpty(startTag);
                   tb->setFramesetOk(false);
               } else if (name.equals("input")) {
                   tb->reconstructFormattingElements(false);
                   Element* el = tb->insertEmpty(startTag);
                   if (!el->attr("type").equalsIgnoreCase("hidden"))
                       tb->setFramesetOk(false);
               } else if (StringUtil::in(name, Constants::InBodyStartMedia, arrayLength(Constants::InBodyStartMedia))) {
                   tb->insertEmpty(startTag);
               } else if (name.equals("hr")) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->insertEmpty(startTag);
                   tb->setFramesetOk(false);
               } else if (name.equals("image")) {
                   if (tb->getFromStack("svg") == NULL) {
                       startTag->setTagName("img");
                       return tb->process(startTag); // change <image> to <img>, unless in svg
                   } else
                       tb->insert(startTag);
               } else if (name.equals("isindex")) {
                   // how much do we care about the early 90s?
                   tb->error(this);
                   if (tb->formElement() != NULL)
                       return false;
                   
                   tb->tokeniser()->setAcknowledgeSelfClosingFlag();
                   processExtraStartTagToken("form", tb);
                   if (startTag->attributes()->hasAttribute("action")) {
                       Element* form = tb->formElement();
                       form->addAttribute("action", startTag->attribute("action"));
                   }
                   
                   processExtraStartTagToken("hr", tb);
                   processExtraStartTagToken("label", tb);
                   // hope you like english.
                   StringRef prompt = startTag->attributes()->hasAttribute("prompt") ?
                                    startTag->attributes()->get("prompt") :
                                    "This is a searchable index. Enter search keywords: ";
                   
                   processExtraCharToken(prompt, tb);
                   
                   // input
                   Attributes inputAttribs(tb->allocator());
                   Attributes* attrsOfStartTag = startTag->attributes();
                   for (size_t i = 0; i < attrsOfStartTag->size(); ++ i) {
                       const Attribute* attr = attrsOfStartTag->get(i);
                       if (!StringUtil::in(attr->key(), Constants::InBodyStartInputAttribs,
                                           arrayLength(Constants::InBodyStartInputAttribs))) {
                           inputAttribs.addAttribute(attr->key(), attr->value());
                       }
                   }
                 
                   inputAttribs.addAttribute("name", "isindex");
                   
                   processExtraToken(CSOUP_NEW3(tb->allocator(), StartTagToken, "input", inputAttribs, tb->allocator()), tb);
                   processExtraEndTagToken("label", tb);
                   processExtraStartTagToken("hr", tb);
                   processExtraEndTagToken("form", tb);
               } else if (name.equals("textarea")) {
                   tb->insert(startTag);
                   // todo: If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to the next one. (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                   tb->setTokeniserState(internal::Rcdata::instance());
                   tb->markInsertionMode();
                   tb->setFramesetOk(false);
                   tb->transition(Text::instance());
               } else if (name.equals("xmp")) {
                   if (tb->inButtonScope("p")) {
                       processExtraEndTagToken("p", tb);
                   }
                   tb->reconstructFormattingElements(false);
                   tb->setFramesetOk(false);
                   handleRawtext(startTag, tb);
               } else if (name.equals("iframe")) {
                   tb->setFramesetOk(false);
                   handleRawtext(startTag, tb);
               } else if (name.equals("noembed")) {
                   // also handle noscript if script enabled
                   handleRawtext(startTag, tb);
               } else if (name.equals("select")) {
                   tb->reconstructFormattingElements(false);
                   tb->insert(startTag);
                   tb->setFramesetOk(false);
                   
                   HtmlTreeBuilderState* state = tb->state();
                   if (state == InTable::instance() || state == InCaption::instance() || state == InTableBody::instance() || state == InRow::instance() || state == InCell::instance())
                       tb->transition(InSelectInTable::instance());
                   else
                       tb->transition(InSelect::instance());
               } else if (StringUtil::in(name, Constants::InBodyStartOptions, arrayLength(Constants::InBodyStartOptions))) {
                   if (tb->currentElement()->tagName().equals("option"))
                       processExtraEndTagToken("option", tb);
                   tb->reconstructFormattingElements(false);
                   tb->insert(startTag);
               } else if (StringUtil::in(name, Constants::InBodyStartRuby, arrayLength(Constants::InBodyStartRuby))) {
                   if (tb->inScope("ruby")) {
                       tb->generateImpliedEndTags(false);
                       if (!tb->currentElement()->tagName().equals("ruby")) {
                           tb->error(this);
                           tb->popStackToBefore(false, "ruby"); // i.e. close up to but not include name
                       }
                       tb->insert(startTag);
                   }
               } else if (name.equals("math")) {
                   tb->reconstructFormattingElements(false);
                   // todo: handle A start tag whose tag name is "math" (i.e. foreign, mathml)
                   tb->insert(startTag);
                   tb->tokeniser()->setAcknowledgeSelfClosingFlag();
               } else if (name.equals("svg")) {
                   tb->reconstructFormattingElements(false);
                   // todo: handle A start tag whose tag name is "svg" (xlink, svg)
                   tb->insert(startTag);
                   tb->tokeniser()->setAcknowledgeSelfClosingFlag();
               } else if (StringUtil::in(name, Constants::InBodyStartDrop, arrayLength(Constants::InBodyStartDrop))) {
                   tb->error(this);
                   return false;
               } else {
                   tb->reconstructFormattingElements(false);
                   tb->insert(startTag);
               }
               break;
           }
           case CSOUP_TOKEN_END_TAG: {
               EndTagToken* endTag = t->asEndTagToken();
               StringRef name = endTag->tagName();
               if (name.equals("body")) {
                   if (!tb->inScope("body")) {
                       tb->error(this);
                       return false;
                   } else {
                       // todo: error if stack contains something not dd, dt, li, optgroup, option, p, rp, rt, tbody, td, tfoot, th, thead, tr, body, html
                       tb->transition(AfterBody::instance());
                   }
               } else if (name.equals("html")) {
                   bool notIgnored = processExtraEndTagToken("body", tb);
                   if (notIgnored)
                       return tb->process(endTag);
               } else if (StringUtil::in(name, Constants::InBodyEndClosers, arrayLength(Constants::InBodyEndClosers))) {
                   if (!tb->inScope(name)) {
                       // nothing to close
                       tb->error(this);
                       return false;
                   } else {
                       tb->generateImpliedEndTags(false);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, name);
                   }
               } else if (name.equals("form")) {
                   Element* currentForm = tb->formElement();
                   tb->setFormElement(NULL, false);
                   if (currentForm == NULL || !tb->inScope(name)) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->generateImpliedEndTags(false);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       // remove currentForm from stack-> will shift anything under up.
                       tb->removeFromStack(currentForm, false);
                   }
               } else if (name.equals("p")) {
                   if (!tb->inButtonScope(name)) {
                       tb->error(this);
                       processExtraStartTagToken(name, tb); // if no p to close, creates an empty <p></p>
                       return tb->process(endTag);
                   } else {
                       tb->generateImpliedEndTags(name);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, name);
                   }
               } else if (name.equals("li")) {
                   if (!tb->inListItemScope(name)) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->generateImpliedEndTags(name);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, name);
                   }
               } else if (StringUtil::in(name, Constants::DdDt, arrayLength(Constants::DdDt))) {
                   if (!tb->inScope(name)) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->generateImpliedEndTags(name);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, name);
                   }
               } else if (StringUtil::in(name, Constants::Headings, arrayLength(Constants::Headings))) {
                   if (!tb->inScope(Constants::Headings, arrayLength(Constants::Headings))) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->generateImpliedEndTags(name);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, Constants::Headings, arrayLength(Constants::Headings));
                   }
               } else if (name.equals("sarcasm")) {
                   // *sigh*
                   return InBodyAnyOtherEndTag(this, t, tb);
               } else if (StringUtil::in(name, Constants::InBodyEndAdoptionFormatters, arrayLength(Constants::InBodyEndAdoptionFormatters))) {
                   // Adoption Agency Algorithm.
               OUTER:
                   for (int i = 0; i < 8; i++) {
                       Element* formatEl = tb->getActiveFormattingElement(name);
                       if (formatEl == NULL)
                           return InBodyAnyOtherEndTag(this, t, tb);
                       else if (!tb->onStack(formatEl)) {
                           tb->error(this);
                           tb->removeFromActiveFormattingElements(formatEl, false);
                           return true;
                       } else if (!tb->inScope(formatEl->tagName())) {
                           tb->error(this);
                           return false;
                       } else if (tb->currentElement() != formatEl)
                           tb->error(this);
                       
                       Element* furthestBlock = NULL;
                       Element* commonAncestor = NULL;
                       bool seenFormattingElement = false;
                       internal::Vector<Element*>* stack = tb->stack();
                       // the spec doesn't limit to < 64, but in degenerate cases (9000+ stack depth) this prevents
                       // run-aways
                       const size_t stackSize = stack->size();
                       for (size_t si = 0; si < stackSize && si < 64; si++) {
                           Element* el = *stack->at(si);
                           if (el == formatEl) {
                               commonAncestor = *stack->at(si - 1);
                               seenFormattingElement = true;
                           } else if (seenFormattingElement && tb->isSpecial(el)) {
                               furthestBlock = el;
                               break;
                           }
                       }
                       if (furthestBlock == NULL) {
                           tb->popStackToClose(false, formatEl->tagName());
                           tb->removeFromActiveFormattingElements(formatEl, false);
                           return true;
                       }
                       
                       // todo: Let a bookmark note the position of the formatting element in the list of active formatting elements relative to the elements on either side of it in the list.
                       // does that mean: int pos of format el in list?
                       Element* node = furthestBlock;
                       Element* lastNode = furthestBlock;
                   INNER:
                       for (int j = 0; j < 3; j++) {
                           if (tb->onStack(node))
                               node = tb->aboveOnStack(node);
                           if (!tb->isInActiveFormattingElements(node)) { // note no bookmark check
                               tb->removeFromStack(node,false);
                               continue;
                           } else if (node == formatEl)
                               break;
  
                           Element* replacement = CSOUP_NEW3(tb->allocator(), Element, node->tagName(), tb->baseUri(), tb->allocator());
                           tb->replaceActiveFormattingElement(node, replacement, false);
                           tb->replaceOnStack(node, replacement, false);
                           node = replacement;
                           
                           if (lastNode == furthestBlock) {
                               // todo: move the aforementioned bookmark to be immediately after the new node in the list of active formatting elements.
                               // not getting how this bookmark both straddles the element above, but is inbetween here...
                           }
                           if (lastNode->parentNode() != NULL) {
                               // don't destroy it ! we;ll
                               lastNode->removeFromParent(false);
                           }
                           node->appendNode(lastNode);
                           lastNode = node;
                       }
                       
                       if (StringUtil::in(commonAncestor->tagName(), Constants::InBodyEndTableFosters,
                                          arrayLength(Constants::InBodyEndTableFosters))) {
                           if (lastNode->parentNode() != NULL)
                               lastNode->removeFromParent(false);
                           tb->insertInFosterParent(lastNode);
                       } else {
                           if (lastNode->parentNode() != NULL)
                               lastNode->removeFromParent(false);
                           
                           commonAncestor->appendNode(lastNode);
                       }
                       
                       Element* adopter = CSOUP_NEW3(tb->allocator(), Element, formatEl->tagName(), tb->baseUri(), tb->allocator());
                       adopter->addAttributes(*formatEl->attributes());
                       
                       for (size_t i = formatEl->childNodeSize(); i > 0; -- i) {
                           Node* c = formatEl->childNode(i - 1);
                           c->removeFromParent(false);
                           // This is very slow
                           adopter->insertNode(0, c);
                       }
                       
                       furthestBlock->appendNode(adopter);
                       tb->removeFromActiveFormattingElements(formatEl, false);
                       // todo: insert the new element into the list of active formatting elements at the position of the aforementioned bookmark.
                       tb->removeFromStack(formatEl, false);
                       tb->insertOnStackAfter(furthestBlock, adopter);
                   }
               //AFTER_OUTER:
               } else if (StringUtil::in(name, Constants::InBodyStartApplets, arrayLength(Constants::InBodyStartApplets))) {
                   if (!tb->inScope("name")) {
                       if (!tb->inScope(name)) {
                           tb->error(this);
                           return false;
                       }
                       tb->generateImpliedEndTags(false);
                       if (!tb->currentElement()->tagName().equals(name))
                           tb->error(this);
                       tb->popStackToClose(false, name);
                       tb->clearFormattingElementsToLastMarker(false);
                   }
               } else if (name.equals("br")) {
                   tb->error(this);
                   processExtraStartTagToken("br", tb);
                   return false;
               } else {
                   return InBodyAnyOtherEndTag(this, t, tb);
               }
               
               break;
           }
           case CSOUP_TOKEN_EOF:
               // todo: error if stack contains something not dd, dt, li, p, tbody, td, tfoot, th, thead, tr, body, html
               // stop parsing
               break;
       }
       return true;
    }

    bool Text::process(Token* t, HtmlTreeBuilder* tb) {
       //TokenDeleter tokenDeleter(t, tb->allocator());
       
       if (t->isCharacterToken()) {
           tb->insert(t->asCharacterToken());
       } else if (t->isEOFToken()) {
           tb->error(this);
           // if current node is script: already started
           tb->pop();
           tb->transition(tb->originalState());
           return tb->process(t);
       } else if (t->isEndTagToken()) {
           // if: An end tag whose tag name is "script" -- scripting nesting level, if evaluating scripts
           tb->pop();
           tb->transition(tb->originalState());
       }
       return true;
    }
    
        bool InTableAnythingElse(HtmlTreeBuilderState* state, Token* t, HtmlTreeBuilder* tb) {
            tb->error(state);
            bool processed = true;
            if (StringUtil::in(tb->currentElement()->tagName(), "table", "tbody", "tfoot", "thead", "tr")) {
                tb->setFosterInserts(true);
                processed = tb->process(t, InBody::instance());
                tb->setFosterInserts(false);
            } else {
                processed = tb->process(t, InBody::instance());
            }
            return processed;
        }
    
       bool InTable::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isCharacterToken()) {
               tb->newPendingTableCharacters(true);
               tb->markInsertionMode();
               tb->transition(InTableText::instance());
               return tb->process(t);
           } else if (t->isCommentToken()) {
               tb->insert(t->asCommentToken());
               return true;
           } else if (t->isDoctypeToken()) {
               tb->error(this);
               return false;
           } else if (t->isStartTagToken()) {
               StartTagToken* startTag = t->asStartTagToken();
               StringRef name = startTag->tagName();
               if (name.equals("caption")) {
                   tb->clearStackToTableContext(false);
                   tb->insertMarkerToFormattingElements();
                   tb->insert(startTag);
                   tb->transition(InCaption::instance());
               } else if (name.equals("colgroup")) {
                   tb->clearStackToTableContext(false);
                   tb->insert(startTag);
                   tb->transition(InColumnGroup::instance());
               } else if (name.equals("col")) {
                   processExtraStartTagToken("colgroup", tb);
                   return tb->process(t);
               } else if (StringUtil::in(name, "tbody", "tfoot", "thead")) {
                   tb->clearStackToTableContext(false);
                   tb->insert(startTag);
                   tb->transition(InTableBody::instance());
               } else if (StringUtil::in(name, "td", "th", "tr")) {
                   processExtraStartTagToken("tbody", tb);
                   return tb->process(t);
               } else if (name.equals("table")) {
                   tb->error(this);
                   bool processed = processExtraEndTagToken("table", tb);
                   if (processed) // only ignored if in fragment
                       return tb->process(t);
               } else if (StringUtil::in(name, "style", "script")) {
                   return tb->process(t, InHead::instance());
               } else if (name.equals("input")) {
                   if (!startTag->attribute("type").equalsIgnoreCase("hidden")) {
                       return InTableAnythingElse(this, t, tb);
                   } else {
                       tb->insertEmpty(startTag);
                   }
               } else if (name.equals("form")) {
                   tb->error(this);
                   if (tb->formElement() != NULL)
                       return false;
                   else {
                       tb->insertForm(startTag, false);
                   }
               } else {
                   return InTableAnythingElse(this, t, tb);
               }
               return true; // todo: check if should return processed http://www.whatwg.org/specs/web-apps/current-work/multipage/tree-construction.html#parsing-main-intable
           } else if (t->isEndTagToken()) {
               EndTagToken* endTag = t->asEndTagToken();
               StringRef name = endTag->tagName();
               static const StringRef errTags[] = {"body", "caption", "col", "colgroup", "html", "tbody", "td", "tfoot", "th", "thead", "tr"};
               
               if (name.equals("table")) {
                   if (!tb->inTableScope(name)) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->popStackToClose(false, "table");
                   }
                   tb->resetInsertionMode();
               } else if (StringUtil::in(name, errTags, arrayLength(errTags))) {
                   tb->error(this);
                   return false;
               } else {
                   return InTableAnythingElse(this, t, tb);
               }
               return true; // todo: as above todo
           } else if (t->isEOFToken()) {
               if (tb->currentElement()->tagName().equals("html"))
                   tb->error(this);
               return true; // stops parsing
           }
           return InTableAnythingElse(this, t, tb);
       }
       
       bool InTableText::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           switch (t->tokenType()) {
               case CSOUP_TOKEN_CHARACTER: {
                   CharacterToken*c = t->asCharacterToken();
                   if (c->data().equals(nullString_)) {
                       tb->error(this);
                       return false;
                   } else {
                       CharacterToken* copy_c = CSOUP_NEW2(tb->allocator(), CharacterToken, c->data(), tb->allocator());
                       tb->pendingTableCharacters()->push(copy_c);
                   }
                   break;
               }
               default: {
                   if (tb->pendingTableCharacters()->size() > 0) {
                       const size_t len = tb->pendingTableCharacters()->size();
                       for (size_t i = 0; i < len; ++ i) {
                           CharacterToken* character = *tb->pendingTableCharacters()->at(i);
                           
                           if (!isWhitespace(character)) {
                               // InTable anything else section:
                               tb->error(this);
                               if (StringUtil::in(tb->currentElement()->tagName(), "table", "tbody", "tfoot", "thead", "tr")) {
                                   tb->setFosterInserts(true);
                                   tb->process(character, InBody::instance());
                                   tb->setFosterInserts(false);
                               } else {
                                   tb->process(character, InBody::instance());
                               }
                           } else
                               tb->insert(character);
                       }
                       tb->newPendingTableCharacters(true);
                   }
                   tb->transition(tb->originalState());
                   return tb->process(t);
               }
           }
           return true;
       }
       
       bool InCaption::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           static const StringRef errStartTags[] = {"caption", "col", "colgroup", "tbody", "td", "tfoot", "th", "thead", "tr"};
           static const StringRef errEndTags[] = {"body", "col", "colgroup", "html", "tbody", "td", "tfoot", "th", "thead", "tr"};
           
           if (t->isEndTagToken() && t->asEndTagToken()->tagName().equals("caption")) {
               EndTagToken* endTag = t->asEndTagToken();
               StringRef name = endTag->tagName();
               if (!tb->inTableScope(name)) {
                   tb->error(this);
                   return false;
               } else {
                   tb->generateImpliedEndTags(false);
                   if (!tb->currentElement()->tagName().equals("caption"))
                       tb->error(this);
                   tb->popStackToClose(false, "caption");
                   tb->clearFormattingElementsToLastMarker(false);
                   tb->transition(InTable::instance());
               }
           } else if ( (t->isStartTagToken() && StringUtil::in(t->asStartTagToken()->tagName(), errStartTags, arrayLength(errStartTags))) ||
                       (t->isEndTagToken() && t->asEndTagToken()->tagName().equals("table"))) {
               tb->error(this);
               bool processed = processExtraEndTagToken("caption", tb);
               if (processed)
                   return tb->process(t);
           } else if (t->isEndTagToken() && StringUtil::in(t->asEndTagToken()->tagName(), errEndTags, arrayLength(errEndTags))) {
               tb->error(this);
               return false;
           } else {
               return tb->process(t, InBody::instance());
           }
           return true;
       }
    
#define InColumnGroupAnythingElse \
    do { \
        bool processed = processExtraEndTagToken("colgroup", tb); \
        if (processed)  \
            return tb->process(t); \
        return true; \
    } while (0)
    
       bool InColumnGroup::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (isWhitespace(t)) {
               tb->insert(t->asCharacterToken());
               return true;
           }
           switch (t->tokenType()) {
               case CSOUP_TOKEN_COMMENT:
                   tb->insert(t->asCommentToken());
                   break;
               case CSOUP_TOKEN_DOCTYPE:
                   tb->error(this);
                   break;
               case CSOUP_TOKEN_START_TAG: {
                   StartTagToken* startTag = t->asStartTagToken();
                   StringRef name = startTag->tagName();
                   if (name.equals("html"))
                       return tb->process(t, InBody::instance());
                   else if (name.equals("col"))
                       tb->insertEmpty(startTag);
                   else
                       InColumnGroupAnythingElse; //return anythingElse(t, tb);
                   break;
               }
               case CSOUP_TOKEN_END_TAG: {
                   EndTagToken* endTag = t->asEndTagToken();
                   StringRef name = endTag->tagName();
                   if (name.equals("colgroup")) {
                       if (tb->currentElement()->tagName().equals("html")) { // frag case
                           tb->error(this);
                           return false;
                       } else {
                           tb->pop();
                           tb->transition(InTable::instance());
                       }
                   } else
                       InColumnGroupAnythingElse; //anythingElse(t, tb);
                   break;
               }
               case CSOUP_TOKEN_EOF: {
                   if (tb->currentElement()->tagName().equals("html"))
                       return true; // stop parsing; frag case
                   else
                       InColumnGroupAnythingElse;//return anythingElse(t, tb);
               }
               default: {
                   InColumnGroupAnythingElse;//return anythingElse(t, tb);
               }
           }
           return true;
       }
       
       bool InTableBody::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           switch (t->tokenType()) {
               case CSOUP_TOKEN_START_TAG: {
                   StartTagToken* startTag = t->asStartTagToken();
                   StringRef name = startTag->tagName();
                   if (name.equals("tr")) {
                       tb->clearStackToTableBodyContext(false);
                       tb->insert(startTag);
                       tb->transition(InRow::instance());
                   } else if (StringUtil::in(name, "th", "td")) {
                       tb->error(this);
                       processExtraStartTagToken("tr", tb);
                       return tb->process(startTag);
                   } else if (StringUtil::in(name, "caption", "col", "colgroup", "tbody", "tfoot", "thead")) {
                       return exitTableBody(t, tb);
                   } else
                       return anythingElse(t, tb);
                   break;
               }
               case CSOUP_TOKEN_END_TAG: {
                   EndTagToken* endTag = t->asEndTagToken();
                   StringRef name = endTag->tagName();
                   static const StringRef errTags[] = {"body", "caption", "col", "colgroup", "html", "td", "th", "tr"};
                   
                   if (StringUtil::in(name, "tbody", "tfoot", "thead")) {
                       if (!tb->inTableScope(name)) {
                           tb->error(this);
                           return false;
                       } else {
                           tb->clearStackToTableBodyContext(false);
                           tb->pop();
                           tb->transition(InTable::instance());
                       }
                   } else if (name.equals("table")) {
                       return exitTableBody(t, tb);
                   } else if (StringUtil::in(name, errTags, arrayLength(errTags))) {
                       tb->error(this);
                       return false;
                   } else
                       return anythingElse(t, tb);
                   break;
               }
               default:
                   return anythingElse(t, tb);
           }
           return true;
       }
       
       bool InTableBody::exitTableBody(Token* t, HtmlTreeBuilder* tb) {
           if (!(tb->inTableScope("tbody") || tb->inTableScope("thead") || tb->inScope("tfoot"))) {
               // frag case
               tb->error(this);
               return false;
           }
           tb->clearStackToTableBodyContext(false);
           processExtraEndTagToken(tb->currentElement()->tagName(), tb);
           return tb->process(t);
       }
       
       bool InTableBody::anythingElse(Token* t, HtmlTreeBuilder* tb) {
           return tb->process(t, InTable::instance());
       }
       
       bool InRow::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isStartTagToken()) {
               StartTagToken* startTag = t->asStartTagToken();
               StringRef name = startTag->tagName();
               
               if (StringUtil::in(name, "th", "td")) {
                   tb->clearStackToTableRowContext(false);
                   tb->insert(startTag);
                   tb->transition(InCell::instance());
                   tb->insertMarkerToFormattingElements();
               } else if (StringUtil::in(name, "caption", "col", "colgroup", "tbody", "tfoot", "thead", "tr")) {
                   return handleMissingTr(t, tb);
               } else {
                   return anythingElse(t, tb);
               }
           } else if (t->isEndTagToken()) {
               EndTagToken* endTag = t->asEndTagToken();
               StringRef name = endTag->tagName();
               
               if (name.equals("tr")) {
                   if (!tb->inTableScope(name)) {
                       tb->error(this); // frag
                       return false;
                   }
                   tb->clearStackToTableRowContext(false);
                   tb->pop(); // tr
                   tb->transition(InTableBody::instance());
               } else if (name.equals("table")) {
                   return handleMissingTr(t, tb);
               } else if (StringUtil::in(name, "tbody", "tfoot", "thead")) {
                   if (!tb->inTableScope(name)) {
                       tb->error(this);
                       return false;
                   }
                   processExtraEndTagToken("tr", tb);
                   return tb->process(t);
               } else if (StringUtil::in(name, "body", "caption", "col", "colgroup", "html", "td", "th")) {
                   tb->error(this);
                   return false;
               } else {
                   return anythingElse(t, tb);
               }
           } else {
               return anythingElse(t, tb);
           }
           return true;
       }
       
    bool InRow::anythingElse(Token* t, HtmlTreeBuilder* tb) {
        return tb->process(t, InTable::instance());
    }
       
    bool InRow::handleMissingTr(Token* t, HtmlTreeBuilder* tb) {
       bool processed = processExtraEndTagToken("tr", tb);
       if (processed)
           return tb->process(t);
       else
           return false;
    }
       
       bool InCell::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isEndTagToken()) {
               EndTagToken* endTag = t->asEndTagToken();
               StringRef name = endTag->tagName();
               
               if (StringUtil::in(name, "td", "th")) {
                   if (!tb->inTableScope(name)) {
                       tb->error(this);
                       tb->transition(InRow::instance()); // might not be in scope if empty: <td /> and processing fake end tag
                       return false;
                   }
                   tb->generateImpliedEndTags(false);
                   if (!tb->currentElement()->tagName().equals(name))
                       tb->error(this);
                   tb->popStackToClose(false, name);
                   tb->clearFormattingElementsToLastMarker(false);
                   tb->transition(InRow::instance());
               } else if (StringUtil::in(name, "body", "caption", "col", "colgroup", "html")) {
                   tb->error(this);
                   return false;
               } else if (StringUtil::in(name, "table", "tbody", "tfoot", "thead", "tr")) {
                   if (!tb->inTableScope(name)) {
                       tb->error(this);
                       return false;
                   }
                   closeCell(tb);
                   return tb->process(t);
               } else {
                   return anythingElse(t, tb);
               }
           } else if (t->isStartTagToken() &&
                      StringUtil::in(t->asStartTagToken()->tagName(),"caption", "col", "colgroup", "tbody") &&
                      StringUtil::in(t->asStartTagToken()->tagName(), "td", "tfoot", "th", "thead", "tr")) {
                          if (!(tb->inTableScope("td") || tb->inTableScope("th"))) {
                              tb->error(this);
                              return false;
                          }
                          closeCell(tb);
                          return tb->process(t);
                      } else {
                          return anythingElse(t, tb);
                      }
           return true;
       }
       
    bool InCell::anythingElse(Token* t, HtmlTreeBuilder* tb) {
        return tb->process(t, InBody::instance());
    }
       
    void InCell::closeCell(HtmlTreeBuilder* tb) {
        if (tb->inTableScope("td"))
            processExtraEndTagToken("td", tb);
        else
            processExtraEndTagToken("th", tb); // only here if th or td in scope
    }
       
       bool InSelect::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           switch (t->tokenType()) {
               case CSOUP_TOKEN_CHARACTER: {
                   CharacterToken*c = t->asCharacterToken();
                   if (c->data().equals(nullString_)) {
                       tb->error(this);
                       return false;
                   } else {
                       tb->insert(c);
                   }
                   break;
               }
               case CSOUP_TOKEN_COMMENT: {
                   tb->insert(t->asCommentToken());
                   break;
               }
               case CSOUP_TOKEN_DOCTYPE: {
                   tb->error(this);
                   return false;
               }
               case CSOUP_TOKEN_START_TAG: {
                   StartTagToken* start = t->asStartTagToken();
                   StringRef name = start->tagName();
                   if (name.equals("html"))
                       return tb->process(start, InBody::instance());
                   else if (name.equals("option")) {
                       processExtraEndTagToken("option", tb);
                       tb->insert(start);
                   } else if (name.equals("optgroup")) {
                       if (tb->currentElement()->tagName().equals("option"))
                           processExtraEndTagToken("option", tb);
                       else if (tb->currentElement()->tagName().equals("optgroup"))
                           processExtraEndTagToken("optgroup", tb);
                       tb->insert(start);
                   } else if (name.equals("select")) {
                       tb->error(this);
                       return processExtraEndTagToken("select", tb);
                   } else if (StringUtil::in(name, "input", "keygen", "textarea")) {
                       tb->error(this);
                       if (!tb->inSelectScope("select"))
                           return false; // frag
                       processExtraEndTagToken("select", tb);
                       return tb->process(start);
                   } else if (name.equals("script")) {
                       return tb->process(t, InHead::instance());
                   } else {
                       return anythingElse(t, tb);
                   }
                   break;
                }
               case CSOUP_TOKEN_END_TAG: {
                   EndTagToken* end = t->asEndTagToken();
                   StringRef name = end->tagName();
                   if (name.equals("optgroup")) {
                       if (tb->currentElement()->tagName().equals("option") && tb->aboveOnStack(tb->currentElement()) != NULL && tb->aboveOnStack(tb->currentElement())->tagName().equals("optgroup"))
                           processExtraEndTagToken("option", tb);
                       if (tb->currentElement()->tagName().equals("optgroup"))
                           tb->pop();
                       else
                           tb->error(this);
                   } else if (name.equals("option")) {
                       if (tb->currentElement()->tagName().equals("option"))
                           tb->pop();
                       else
                           tb->error(this);
                   } else if (name.equals("select")) {
                       if (!tb->inSelectScope(name)) {
                           tb->error(this);
                           return false;
                       } else {
                           tb->popStackToClose(false, name);
                           tb->resetInsertionMode();
                       }
                   } else
                       return anythingElse(t, tb);
                   break;
               }
               case CSOUP_TOKEN_EOF:
                   if (!tb->currentElement()->tagName().equals("html"))
                       tb->error(this);
                   break;
               default:
                   return anythingElse(t, tb);
           }
           return true;
       }
       
       bool InSelect::anythingElse(Token* t, HtmlTreeBuilder* tb) {
           tb->error(this);
           return false;
       }
       
       bool InSelectInTable::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isStartTagToken() && StringUtil::in(t->asStartTagToken()->tagName(), "caption", "table", "tbody", "tfoot") &&
               StringUtil::in(t->asStartTagToken()->tagName(), "thead", "tr", "td", "th")) {
               tb->error(this);
               processExtraEndTagToken("select", tb);
               return tb->process(t);
           } else if (t->isEndTagToken() && StringUtil::in(t->asEndTagToken()->tagName(), "caption", "table", "tbody", "tfoot") && StringUtil::in(t->asEndTagToken()->tagName(), "thead", "tr", "td", "th")) {
               tb->error(this);
               if (tb->inTableScope(t->asEndTagToken()->tagName())) {
                   processExtraEndTagToken("select", tb);
                   return (tb->process(t));
               } else
                   return false;
           } else {
               return tb->process(t, InSelect::instance());
           }
       }
       
       bool AfterBody::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (isWhitespace(t)) {
               return tb->process(t, InBody::instance());
           } else if (t->isCommentToken()) {
               tb->insert(t->asCommentToken()); // into html node
           } else if (t->isDoctypeToken()) {
               tb->error(this);
               return false;
           } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html")) {
               return tb->process(t, InBody::instance());
           } else if (t->isEndTagToken() && t->asEndTagToken()->tagName().equals("html")) {
               if (tb->isFragmentParsing()) {
                   tb->error(this);
                   return false;
               } else {
                   tb->transition(AfterAfterBody::instance());
               }
           } else if (t->isEOFToken()) {
               // chillax! we're done
           } else {
               tb->error(this);
               tb->transition(InBody::instance());
               return tb->process(t);
           }
           return true;
       }
       
       bool InFrameset::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (isWhitespace(t)) {
               tb->insert(t->asCharacterToken());
           } else if (t->isCommentToken()) {
               tb->insert(t->asCommentToken());
           } else if (t->isDoctypeToken()) {
               tb->error(this);
               return false;
           } else if (t->isStartTagToken()) {
               StartTagToken* start = t->asStartTagToken();
               StringRef name = start->tagName();
               if (name.equals("html")) {
                   return tb->process(start, InBody::instance());
               } else if (name.equals("frameset")) {
                   tb->insert(start);
               } else if (name.equals("frame")) {
                   tb->insertEmpty(start);
               } else if (name.equals("noframes")) {
                   return tb->process(start, InHead::instance());
               } else {
                   tb->error(this);
                   return false;
               }
           } else if (t->isEndTagToken() && t->asEndTagToken()->tagName().equals("frameset")) {
               if (tb->currentElement()->tagName().equals("html")) { // frag
                   tb->error(this);
                   return false;
               } else {
                   tb->pop();
                   if (!tb->isFragmentParsing() && !tb->currentElement()->tagName().equals("frameset")) {
                       tb->transition(AfterFrameset::instance());
                   }
               }
           } else if (t->isEOFToken()) {
               if (!tb->currentElement()->tagName().equals("html")) {
                   tb->error(this);
                   return true;
               }
           } else {
               tb->error(this);
               return false;
           }
           return true;
       }
       
       bool AfterFrameset::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (isWhitespace(t)) {
               tb->insert(t->asCharacterToken());
           } else if (t->isCommentToken()) {
               tb->insert(t->asCommentToken());
           } else if (t->isDoctypeToken()) {
               tb->error(this);
               return false;
           } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html")) {
               return tb->process(t, InBody::instance());
           } else if (t->isEndTagToken() && t->asEndTagToken()->tagName().equals("html")) {
               tb->transition(AfterAfterFrameset::instance());
           } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("noframes")) {
               return tb->process(t, InHead::instance());
           } else if (t->isEOFToken()) {
               // cool your heels, we're complete
           } else {
               tb->error(this);
               return false;
           }
           return true;
       }
       
       bool AfterAfterBody::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isCommentToken()) {
               tb->insert(t->asCommentToken());
           } else if (t->isDoctypeToken() || isWhitespace(t) || (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html"))) {
               return tb->process(t, InBody::instance());
           } else if (t->isEOFToken()) {
               // nice work chuck
           } else {
               tb->error(this);
               tb->transition(InBody::instance());
               return tb->process(t);
           }
           return true;
       }
       
       bool AfterAfterFrameset::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           if (t->isCommentToken()) {
               tb->insert(t->asCommentToken());
           } else if (t->isDoctypeToken() || isWhitespace(t) || (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("html"))) {
               return tb->process(t, InBody::instance());
           } else if (t->isEOFToken()) {
               // nice work chuck
           } else if (t->isStartTagToken() && t->asStartTagToken()->tagName().equals("noframes")) {
               return tb->process(t, InHead::instance());
           } else {
               tb->error(this);
               return false;
           }
           return true;
       }
       
       bool ForeignContent::process(Token* t, HtmlTreeBuilder* tb) {
           //TokenDeleter tokenDeleter(t, tb->allocator());
           
           return true;
           // todo: implement. Also; how do we get here?
       }
    

}

