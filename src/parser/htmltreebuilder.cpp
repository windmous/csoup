//
//  htmltreebuilder.cpp
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//
#include "htmltreebuilder.h"
#include "token.h"
#include "tokeniser.h"
#include "tokeniserstate.h"
#include "util/stringutil.h"
#include "../selector/elementsref.h"
#include "../nodes/element.h"
#include "../nodes/document.h"
#include "../internal/list.h"
#include "htmltreebuilderstate.h"
#include "formelement.h"
#include "parseerrorlist.h"
#include "parseerror.h"

namespace csoup {
    using namespace internal;
    
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
    framesetOk_(true), fosterInserts_(false), fragmentParsing_(false), formElement_(NULL), allocator_(allocator) {
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
                tokeniser_->transition(internal::Rcdata::instance());
            else if (StringUtil::in(contextTag, "iframe", "noembed", "noframes", "style", "xmp"))
                tokeniser_->transition(RawText::instance());
            else if (contextTag.equals("script"))
                tokeniser_->transition(ScriptData::instance());
            else if (contextTag.equals(("noscript")))
                tokeniser_->transition(Data::instance()); // if scripting enabled, rawtext
            else if (contextTag.equals("plaintext"))
                tokeniser_->transition(Data::instance());
            else
                tokeniser_->transition(Data::instance()); // default
            
            root = doc_->appendElement("html");
            stack_->push(root);
            resetInsertionMode();
            
            // setup form element to nearest form on context (up ancestor chain). ensures form controls are associated
            // with form correctly
            ElementsRef contextChain(allocator);
            context->parents(&contextChain);
            contextChain.insert(0, context);
            
            for (size_t i = 0; i < contextChain.size(); ++ i) {
                if (contextChain.get(i)->type() == CSOUP_NODE_FORMELEMENT) {
                    formElement_ = (FormElement*)contextChain.get(i);
                    break;
                }
            }
        
        }
        
        runParser();
        if (context != NULL)
            return NULL;//root->childNodes();
        else
            return NULL;//doc.childNodes();
    }
    
    bool HtmlTreeBuilder::process(Token *token) {
        CSOUP_DELETE(allocator(), currentToken_);
        currentToken_ = token;
        return state_->process(token, this);
    }
    
    bool HtmlTreeBuilder::process(Token* token, HtmlTreeBuilderState* state) {
        return process(token);
    }
    
    void HtmlTreeBuilder::maybeSetBaseUri(csoup::Element *base) {
        if (baseUriSetFromDoc_) {
            return ;
        }
        
        // Do nothing
    }
    
    
    void HtmlTreeBuilder::error(HtmlTreeBuilderState *state) {
        if (errors_->notFull()) {
            new (errors_->appendError()) ParseError(0, "Unexpected token", allocator());
        }
    }
    
    void HtmlTreeBuilder::insertNode(csoup::Node *node) {
        if (stack_->size() == 0) {
            doc_->appendNode(node);
        } else if (fosterInserts()) {
            insertInFosterParent(node);
        } else {
            currentElement()->appendNode(node);
        }
        
        if (node->type() == CSOUP_NODE_ELEMENT && ((Element*)node)->tag()->formListed()) {
            if (formElement_) {
                formElement_->appendElementToForm((Element*)node);
            }
        }
    }
    
    Element* HtmlTreeBuilder::pop() {
        Element* ret = *stack_->back();
        stack_->pop();
        
        return ret;
    }

    void HtmlTreeBuilder::push(Element* el) {
        stack_->push(el);
    }
    
    bool HtmlTreeBuilder::onStack(csoup::Element *el) {
        return isElementInQueue(stack_, el);
    }
    
    bool HtmlTreeBuilder::isElementInQueue(internal::List<Element*> *queue, csoup::Element *element) {
        for (size_t i = 0; i < queue->size(); ++ i) {
            if (element == *queue->at(i)) {
                return true;
            }
        }
        
        return false;
    }
    
    Element* HtmlTreeBuilder::getFromStack(const csoup::StringRef &elName) {
        if (stack_->size() == 0) return NULL;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if ((*stack_->at(i - 1))->tagName().equals(elName)) {
                return *stack_->at(i - 1);
            }
        }
        
        return NULL;
    }
    
    bool HtmlTreeBuilder::removeFromStack(csoup::Element *el, bool del) {
        if (stack_->size() == 0) return false;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (*stack_->at(i - 1) == el) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                return true;
            }
        }
        
        return false;
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const csoup::StringRef &s1) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if ((*stack_->at(i - 1))->tagName().equals(s1)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const StringRef &s1, const StringRef &s2) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (StringUtil::in((*stack_->at(i - 1))->tagName(), s1, s2)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const StringRef &s1, const StringRef &s2, const StringRef &s3) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (StringUtil::in((*stack_->at(i - 1))->tagName(), s1, s2, s3)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const StringRef &s1, const StringRef &s2, const StringRef &s3,
                                          const StringRef& s4) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (StringUtil::in((*stack_->at(i - 1))->tagName(), s1, s2, s3, s4)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const StringRef &s1, const StringRef &s2, const StringRef &s3,
                                          const StringRef& s4, const StringRef& s5) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (StringUtil::in((*stack_->at(i - 1))->tagName(), s1, s2, s3, s4, s5)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToClose(bool del, const csoup::StringRef *elName, size_t cnt) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (StringUtil::in((*stack_->at(i - 1))->tagName(), elName, cnt)) {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::popStackToBefore(bool del, const csoup::StringRef &elName) {
        if (stack_->size() == 0) return ;
        
        for (size_t i = stack_->size(); i > 0; -- i) {
            if ((*stack_->at(i - 1))->tagName().equals(elName)) {
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), (*stack_->at(i - 1)));
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::clearStackToTableContext(bool del) {
        clearStackToContext(del, "table");
    }
    
    void HtmlTreeBuilder::clearStackToTableBodyContext(bool del) {
        clearStackToContext(del, "tbody", "tfoot", "thead");
    }
    
    void HtmlTreeBuilder::clearStackToTableRowContext(bool del) {
        clearStackToContext(del, "tr");
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const csoup::StringRef &n1) {
        if (stack_->empty()) return ;
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* el = *stack_->at(i - 1);
            if (StringUtil::in(el->tagName(), n1) || el->tagName().equals("html")) {
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), el);
                stack_->remove(i - 1);
            }
        }
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const csoup::StringRef &n1, const csoup::StringRef &n2) {
        StringRef names[] = {n1, n2};
        clearStackToContext(del, names, arrayLength(names));
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const StringRef &n1, const StringRef &n2, const StringRef &n3) {
        StringRef names[] = {n1, n2, n3};
        clearStackToContext(del, names, arrayLength(names));
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const StringRef &n1, const StringRef &n2, const StringRef &n3,
                                              const StringRef& n4) {
        StringRef names[] = {n1, n2, n3, n4};
        clearStackToContext(del, names, arrayLength(names));
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const StringRef &n1, const StringRef &n2, const StringRef &n3,
                                              const StringRef& n4, const StringRef& n5) {
        StringRef names[] = {n1, n2, n3, n4, n5};
        clearStackToContext(del, names, arrayLength(names));
    }
    
    void HtmlTreeBuilder::clearStackToContext(bool del, const csoup::StringRef *n1, size_t cnt) {
        if (stack_->empty()) return ;
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* el = *stack_->at(i - 1);
            if (StringUtil::in(el->tagName(), n1, cnt) || el->tagName().equals("html")) {
                break;
            } else {
                if (del) CSOUP_DELETE(allocator(), el);
                stack_->remove(i - 1);
            }
        }
    }
    
    Element* HtmlTreeBuilder::aboveOnStack(csoup::Element *el) {
        CSOUP_ASSERT(onStack(el));
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* next = *stack_->at(i - 1);
            if (next == el) {
                return (i - 1 > 0) ? *stack_->at(i - 2) : NULL;
            }
        }
        
        return NULL;
    }
    
    void HtmlTreeBuilder::insertOnStackAfter(csoup::Element *after, csoup::Element *in) {
        for (size_t i = stack_->size(); i > 0; -- i) {
            if (*stack_->at(i - 1) == after) {
                stack_->insert(i, in);
                return ;
            }
        }
        
        CSOUP_ASSERT(false);
    }
    
    void HtmlTreeBuilder::replaceOnStack(csoup::Element *out, csoup::Element *in, bool del) {
        replaceInQueue(stack_, out, in, del);
    }
    
    void HtmlTreeBuilder::replaceInQueue(internal::List<Element *> *queue, Element *out, Element *in, bool del) {
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* cur = *stack_->at(i - 1);
            if (cur == out) {
                if (del) CSOUP_DELETE(allocator(), cur);
                *stack_->at(i - 1) = in;
            }
        }
    }
    
    void HtmlTreeBuilder::resetInsertionMode() {
        bool last = false;
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* node = *stack_->at(i);
            if (i - 1 == 0) {
                last = true;
                node = contextElement_;
            }
            
            StringRef name(node->tagName());
            if (name.equals("select")) {
                transition(InSelect::instance());
                break;
                
                ////////////////////////
                // There is a bug!
            } else if (name.equals("td") || (name.equals("td") && !last)) {
                transition(InCell::instance());
                break;
            } else if (name.equals("tr")) {
                transition(InRow::instance());
                break;
            } else if (name.equals("tbody") || name.equals("thead") || name.equals("tfoot")) {
                transition(InTableBody::instance());
                break;
            } else if (name.equals("caption")) {
                transition(InCaption::instance());
                break;
            } else if (name.equals("colgroup")) {
                transition(InColumnGroup::instance());
                break;
            } else if (name.equals("table")) {
                transition(InTable::instance());
                break;
            } else if (name.equals("head")) {
                transition(InBody::instance());
                break;
            } else if (name.equals("body")) {
                transition(InBody::instance());
                break;
            } else if (name.equals("frameset")) {
                transition(InFrameset::instance());
                break;
            } else if (name.equals("html")) {
                transition(BeforeHead::instance());
                break;
            } else if (last) {
                transition(InBody::instance());
                break;
            }
        }
    }
    
    bool HtmlTreeBuilder::inSpecificScope(const csoup::StringRef &targetName, const csoup::StringRef *baseTypes, size_t baseTypeLen, const csoup::StringRef *extraTypes, size_t extraTypeLen) {
        StringRef targetNames[] = {targetName};
        return inSpecificScope(targetNames, 1, baseTypes, baseTypeLen, extraTypes, extraTypeLen);
    }
    
    bool HtmlTreeBuilder::inSpecificScope(const StringRef* targetName, size_t targetNameLen, const StringRef *baseTypes, size_t baseTypeLen, const StringRef *extraTypes, size_t extraTypeLen) {
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* el = *stack_->at(i - 1);
            StringRef elName = el->tagName();
            
            if (StringUtil::in(elName, targetName, targetNameLen)) {
                return true;
            }
            
            if (StringUtil::in(elName, baseTypes, baseTypeLen)) {
                return false;
            }
            
            if (extraTypes != NULL && extraTypeLen != 0 && StringUtil::in(elName, extraTypes, extraTypeLen)) {
                return false;
            }
        }
        
        CSOUP_ASSERT(false);
        return false;
    }
    
    bool HtmlTreeBuilder::inScope(const csoup::StringRef &targetName) {
        return inScope(targetName, NULL, 0);
    }
    
    bool HtmlTreeBuilder::inScope(const csoup::StringRef &targetName, const csoup::StringRef *extras, size_t len) {
        return inSpecificScope(targetName, TagsSearchInScope, arrayLength(TagsSearchInScope), extras, len);
    }
    
    bool HtmlTreeBuilder::inScope(const csoup::StringRef *targetNames, size_t len) {
        return inSpecificScope(targetNames, len, TagsSearchInScope, arrayLength(TagsSearchInScope), NULL, 0);
    }
    
    bool HtmlTreeBuilder::inListItemScope(const csoup::StringRef &targetName) {
        return inScope(targetName, TagSearchList, arrayLength(TagSearchList));
    }
    
    bool HtmlTreeBuilder::inButtonScope(const csoup::StringRef &targetName) {
        return inScope(targetName, TagSearchButton, arrayLength(TagSearchButton));
    }
    
    bool HtmlTreeBuilder::inTableScope(const csoup::StringRef &targetName) {
        return inSpecificScope(targetName, TagSearchTableScope, arrayLength(TagSearchTableScope), NULL, 0);
    }
    
    bool HtmlTreeBuilder::inSelectScope(const csoup::StringRef &targetName) {
        for (size_t i = stack_->size(); i > 0; -- i) {
            Element* el = *stack_->at(i);
            StringRef elName = el->tagName();
            
            if (elName.equals(targetName)) return true;
            if (!StringUtil::in(elName, TagSearchSelectScope, arrayLength(TagSearchSelectScope))) {
                return false;
            }
        }
        
        return false;
    }
    
    void HtmlTreeBuilder::setHeadElement(csoup::Element *headElement, bool del) {
        if (del) CSOUP_DELETE(allocator(), headElement_);
        headElement_ = headElement;
    }
    
    void HtmlTreeBuilder::setFormElement(csoup::FormElement *formElement, bool del) {
        if (del) CSOUP_DELETE(allocator(), formElement_);
        formElement_ = formElement;
    }
    
    void HtmlTreeBuilder::newPendingTableCharacters() {
        if (pendingTableCharacters_) CSOUP_DELETE(allocator(), pendingTableCharacters_);
        pendingTableCharacters_ = CSOUP_NEW1(allocator(), internal::List<CharacterToken*>, allocator());
    }
    
    void HtmlTreeBuilder::setPendingTableCharacters(internal::List<CharacterToken*> *pendingTableCharacters, bool del) {
        if (del && pendingTableCharacters_) CSOUP_DELETE(allocator(), pendingTableCharacters_);
        pendingTableCharacters_ = pendingTableCharacters;
    }
    
    void HtmlTreeBuilder::generateImpliedEndTags(const csoup::StringRef &excludeTag, bool del) {
        while ((excludeTag.size() != 0 && !currentElement()->tagName().equals(excludeTag)) &&
               StringUtil::in(currentElement()->tagName(), TagSearchEndTags, arrayLength(TagSearchEndTags))) {
            if (del) CSOUP_DELETE(allocator(), pop());
            else pop();
        }
    }
    
    void HtmlTreeBuilder::generateImpliedEndTags(bool del) {
        generateImpliedEndTags(StringRef(""), del);
    }
    
    bool HtmlTreeBuilder::isSpecial(const csoup::Element *el) {
        return StringUtil::in(el->tagName(), TagSearchSpecial, arrayLength(TagSearchSpecial));
    }
    
    void HtmlTreeBuilder::pushActiveFormattingElements(csoup::Element *in, bool del) {
        int numSeen = 0;
        for (size_t i = formattingElements_->size(); i > 0; -- i) {
            Element* el = *formattingElements_->at(i - 1);
            if (el == NULL)
                break;
            
            if (isSameFormattingElement(in, el))
                numSeen ++;
            
            if (numSeen == 3) {
                if (del) CSOUP_DELETE(allocator(), *formattingElements_->at(i - 1));
                formattingElements_->remove(i - 1);
                break;
            }
        }
        
        formattingElements_->push(in);
    }
    
    bool HtmlTreeBuilder::isSameFormattingElement(csoup::Element *a, csoup::Element *b) {
        return a->tagName().equals(b->tagName()) && a->attributes()->equals(*b->attributes());
    }
    
    void HtmlTreeBuilder::reconstructFormattingElements(bool del) {
        size_t size = formattingElements_->size();
        if (size == 0 || *formattingElements_->back() == NULL || onStack(*formattingElements_->back()))
            return ;
        
        Element* entry = *formattingElements_->back();
        int pos = size - 1;
        bool skip = false;
        while (true) {
            if (pos == 0) {
                skip = true;
                break;
            }
            
            entry = *formattingElements_->at(--pos);
            if (entry == NULL || onStack(entry)) {
                break;
            }
        }
        
        while (true) {
            if (!skip) {
                entry = *formattingElements_->at(++ pos);
            }
            
            CSOUP_ASSERT(entry != NULL);
            skip = false;
            Element* newEl = insert(entry->tagName());
            
            const Attributes* attrs = entry->attributes();
            for (size_t i = 0; i < attrs->size(); ++ i) {
                newEl->addAttribute(attrs->get(i)->key(), attrs->get(i)->value());
            }
            
            formattingElements_->insert(pos, newEl);
            if (del) CSOUP_DELETE(allocator(), *formattingElements_->at(pos + 1));
            formattingElements_->remove(pos + 1);
            
            if (pos == size - 1)
                break;
        }
    }
    
    void HtmlTreeBuilder::clearFormattingElementsToLastMarker(bool del) {
        while (!formattingElements_->empty()) {
            Element* el = *formattingElements_->back();
            if (del) CSOUP_DELETE(allocator(), el);
            formattingElements_->pop();
            if (el == NULL) break;
        }
    }
    
    void HtmlTreeBuilder::removeFromActiveFormattingElements(csoup::Element *el, bool del) {
        for (size_t i = formattingElements_->size(); i > 0; -- i) {
            Element* next = *formattingElements_->at(i - 1);
            if (el == next) {
                if (del) CSOUP_DELETE(allocator(), next);
                formattingElements_->remove(i - 1);
                break;
            }
        }
    }
    
    bool HtmlTreeBuilder::isInActiveFormattingElements(csoup::Element *el) {
        return isElementInQueue(formattingElements_, el);
    }
    
    Element* HtmlTreeBuilder::getActiveFormattingElement(const csoup::StringRef &nodeName) {
        for (size_t i = formattingElements_->size(); i > 0; -- i) {
            Element* next = *formattingElements_->at(i - 1);
            if (next == NULL) {
                break;
            } else if (next->tagName().equals(nodeName)) {
                return next;
            }
        }
        
        return NULL;
    }
    
    void HtmlTreeBuilder::replaceActiveFormattingElement(csoup::Element *out, csoup::Element *in, bool del) {
        replaceInQueue(formattingElements_, out, in, del);
    }
    
    void HtmlTreeBuilder::insertMarkerToFormattingElements() {
        formattingElements_->push(NULL);
    }
    
    void HtmlTreeBuilder::insertInFosterParent(csoup::Node *in) {
        Element* fosterParent = NULL;
        Element* lastTable = getFromStack("table");
        bool isLastTableParent = false;
        if (lastTable != NULL) {
            if (lastTable->parentNode() != NULL) {
                fosterParent = lastTable->parentNode();
                isLastTableParent = true;
            } else {
                fosterParent = aboveOnStack(lastTable);
            }
        } else {
            fosterParent = *stack_->at(0);
        }
        
        if (isLastTableParent) {
            CSOUP_ASSERT(lastTable != NULL);
            lastTable->before(in);
        } else {
            fosterParent->appendNode(in);
        }
    }
}
