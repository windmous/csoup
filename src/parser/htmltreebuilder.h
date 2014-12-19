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
#include "../util/stringref.h"

// TODO:
//      1. Methods about creating/inserting/removing nodes should be modified to be
//            consistent with internal module in style
//      2. Some methods about internal should be made invisible to user. But letting
//          HtmlTreeBuilder invisible is an alternative.
namespace csoup {
    class CharacterToken;
    class HtmlTreeBuilderState;
    class Element;
    class Allocator;
    class StartTagToken;
    class Tag;
    class Node;
    class TagToken;
    class StartTagToken;
    class EndTagToken;
    class Document;
    class CommentToken;
    class HtmlTreeBuilderState;
    class CharacterToken;
    class FormElement;
    
    namespace internal {
        template <typename T>
        class List;
        
        template <typename T>
        class Vector;
    };
    
    class HtmlTreeBuilder : public TreeBuilder {
    public:
        HtmlTreeBuilder(Allocator* allocator);
        
        // Don't relase elements for it's used in Document.
        ~HtmlTreeBuilder();
        
        Document* parse(const StringRef& input, const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator);
        
        // Usesr should mever invoke this
        internal::Vector<Node>* parseFragment(const StringRef& inputFragment, Element* context, const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator);
        
        bool process(Token* token);
        
        bool process(Token* token, HtmlTreeBuilderState* state);
        
        void transition(HtmlTreeBuilderState* state) {
            state_ = state;
        }
        
        HtmlTreeBuilderState* state() {
            return state_;
        }
        
        void markInsertionMode() {
            originalState_ = state_;
        }
        
        HtmlTreeBuilderState* originalState() {
            return originalState_;
        }
        
        void setFramesetOk(bool framesetOk) {
            framesetOk_ = framesetOk;
        }
        
        bool framesetOk() const {
            return framesetOk_;
        }
        
        Document* document() {
            return doc_;
        }
        
        void maybeSetBaseUri(Element* base);
        
        bool isFragmentParsing() const {
            return fragmentParsing_;
        }
        
        void error(HtmlTreeBuilderState* state);
        Element* insert(StartTagToken* startTag);
        
        Element* insert(const StringRef& startTagName);
        
        void insert(Element* el);
        
        Element* insertEmpty(StartTagToken* startTag);
        
        // inserrtForm;
        
        void insert(CommentToken* commentToken);
        void insert(CharacterToken* characterToken);
        
        Element* pop();
        
        ///////////////////////////////////////////////////////////////////////
        // Methods about creating/inserting/removing nodes should be modified to
        // be consistent with other part in style
    
        void push(Element* element);
        
        bool onStack(Element* el);
        
        Element* getFromStack(const StringRef& elName);
        
        bool removeFromStack(Element* el, bool del);
        
        void popStackToClose(bool del, const StringRef& s1);
        void popStackToClose(bool del, const StringRef& s1, const StringRef& s2);
        void popStackToClose(bool del, const StringRef& s1, const StringRef& s2, const StringRef& s3);
        void popStackToClose(bool del, const StringRef& s1, const StringRef& s2, const StringRef& s3, const StringRef& s4);
        void popStackToClose(bool del, const StringRef& s1, const StringRef& s2, const StringRef& s3, const StringRef& s4,
                             const StringRef& s5);
        void popStackToClose(bool del, const StringRef& s1, const StringRef& s2, const StringRef& s3, const StringRef& s4,
                             const StringRef& s5, const StringRef& s6);
        void popStackToClose(bool del, const StringRef* elName, size_t cnt);
        
        void popStackToBefore(bool del, const StringRef& elName);
        
        void clearStackToTableContext(bool del);
        
        void clearStackToTableBodyContext(bool del);
        
        void clearStackToTableRowContext(bool del);
        
        Element* aboveOnStack(Element* el);
        
        void insertOnStackAfter(Element* after, Element* in);
        
        void replaceOnStack(Element* out, Element* in, bool del);
        
        void replaceInQueue(internal::List<Element*>* queue, Element* out, Element* in, bool del);
        
        void resetInsertionMode();
        
        bool inScope(const StringRef* targetNames, size_t len);
        
        bool inScope(const StringRef& targetName);
        
        bool inScope(const StringRef& targetName, const StringRef* extras, size_t len);
        
        bool inListItemScope(const StringRef& targetName);
        
        bool inButtonScope(const StringRef& targetName);
        
        bool inTableScope(const StringRef& targetName);
        
        bool inSelectScope(const StringRef& targetName);
        
        void setHeadElement(Element* headElement, bool del);
        
        Element* headElement() {
            return headElement_;
        }
        
        bool fosterInserts() const {
            return fosterInserts_;
        }
        
        void setFosterInserts(bool fosterInserts) {
            fosterInserts = fosterInserts;
        }
        
        FormElement* formElement() {
            return formElement_;
        }
        
        void setFormElement(FormElement* formElement, bool del);
        
        void newPendingTableCharacters();
        
        internal::List<CharacterToken*>* pendingTableCharacters() {
            return pendingTableCharacters_;
        }
        
        // M
        void setPendingTableCharacters(internal::List<CharacterToken*>* pendingTableCharacters, bool del);
        
        void generateImpliedEndTags(const StringRef& excludeTag, bool del);
        
        void generateImpliedEndTags(bool del);
        
        bool isSpecial(const Element* el);
        
        // active formatting elements
        void pushActiveFormattingElements(Element* in, bool del);
        
        void reconstructFormattingElements(bool del);
        
        void clearFormattingElementsToLastMarker(bool del);
        
        void removeFromActiveFormattingElements(Element* el, bool del);
        
        bool isInActiveFormattingElements(Element* el);
        
        Element* getActiveFormattingElement(const StringRef& nodeName);
        
        void replaceActiveFormattingElement(Element* out, Element* in, bool del);
        
        void insertMarkerToFormattingElements();
        
        void insertInFosterParent(Node* in);
        
        Allocator* allocator() {
            return allocator_;
        }
        
    private:
        void insertNode(Node* node);
        bool isElementInQueue(internal::List<Element*>* queue, Element* element);
        void replaceInQueue(internal::List<Element>* queue, Element* out, Element* in);
        bool isSameFormattingElement(Element* a, Element* b);

        void clearStackToContext(bool del, const StringRef& n1);
        void clearStackToContext(bool del, const StringRef& n1, const StringRef& n2);
        void clearStackToContext(bool del, const StringRef& n1, const StringRef& n2, const StringRef& n3);
        void clearStackToContext(bool del, const StringRef& n1, const StringRef& n2, const StringRef& n3, const StringRef& n4);
        void clearStackToContext(bool del, const StringRef& n1, const StringRef& n2, const StringRef& n3, const StringRef& n4,
                                 const StringRef& n5);
        void clearStackToContext(bool del, const StringRef* n1, size_t cnt);
        
        bool inSpecificScope(const StringRef* targetName, size_t targetNameLen, const StringRef* baseTypes, size_t baseTypeLen,
                            const StringRef* extraTypes, size_t extraTypeLen);
        
        bool inSpecificScope(const StringRef& targetName, const StringRef* baseTypes, size_t baseTypeLen,
                             const StringRef* extraTypes, size_t extraTypeLen);

        
//        bool inSpecificScope(const StringRef& targetNames, StringRef* baseTypes, size_t baseTypeLen,
//                             StringRef* extraTypes, size_t extraTypeLen);

        static const StringRef TagsScriptStyle[];
        static const StringRef TagsSearchInScope[];
        static const StringRef TagSearchList[];
        static const StringRef TagSearchButton[];
        static const StringRef TagSearchTableScope[];
        static const StringRef TagSearchSelectScope[];
        static const StringRef TagSearchEndTags[];
        static const StringRef TagSearchSpecial[];
        
        // Never try to release two guys below. They refered to
        // static members
        HtmlTreeBuilderState* state_;
        HtmlTreeBuilderState* originalState_;
        
        bool baseUriSetFromDoc_;
        Element* headElement_;
        FormElement* formElement_;
        Element* contextElement_;
        
        // these containers are just references
        internal::List<Element*>* formattingElements_;
        internal::List<CharacterToken*>* pendingTableCharacters_;
        
        bool framesetOk_;
        bool fosterInserts_;
        bool fragmentParsing_;
        
        Allocator* allocator_;
    };
}

#endif // CSOUP_HTML_TREEBUILDER_H_
