//
//  treebuilder.h
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_TREEBUILDER_H_
#define CSOUP_TREEBUILDER_H_
#include "../util/stringref.h"

namespace csoup {
    class String;
    class Allocator;
    class Element;
    class CharacterReader;
    class Document;
    class Tokeniser;
    class ParseErrorList;
    class Token;

    
    namespace internal {
        template <class T>
        class Vector;
        
        class TokeniserState;
    }
    
    class TreeBuilder {
    public:
        TreeBuilder();
        virtual ~TreeBuilder();
        
        // errors should never be NULL
        virtual Document* parse(const StringRef& input, const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator) {
            initialiseParse(input, baseUri, errors, allocator);
            runParser();
            
            return doc_;
        }
        
        void setTokeniserState(internal::TokeniserState* state);
        
        internal::Vector<Element*>* stack() {
            return stack_;
        }

        Element* currentElement();
        
        Tokeniser* tokeniser() {
            return tokeniser_;
        }
        
        StringRef baseUri() const;

    protected:
        virtual bool process(Token* token) = 0;

        
        // we make the members have protected privileges, which break the rule
        // of encapsulation in OOP
        
        Allocator* allocator_;
        
        // these are resources needed to be destroied
        CharacterReader* reader_;
        Tokeniser* tokeniser_;
        internal::Vector<Element*>* stack_; // the stack of open elements
        Token* currentToken_; // currentToken is used only for error tracking.
        
        // don't destroy these two guy!
        Document* doc_; // current doc we are building into
        ParseErrorList* errors_; // null when not tracking errors
        String* baseUri_;
        
        void initialiseParse(const StringRef& input, const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator);
        
        void freeResources();
        
        void runParser();
    };
}

#endif // CSOUP_TREEBUILDER_H_
