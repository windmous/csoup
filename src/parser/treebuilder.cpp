//
//  treebuilder.cpp
//  csoup
//
//  Created by mac on 12/15/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "../util/common.h"
#include "../util/stringref.h"
#include "../util/stringbuffer.h"
#include "../util/csoup_string.h"
#include "../internal/list.h"
#include "../nodes/document.h"
#include "characterreader.h"
#include "parseerror.h"
#include "parseerrorlist.h"
#include "token.h"
#include "tokeniser.h"
#include "treebuilder.h"

namespace csoup {
    TreeBuilder::TreeBuilder() :
    allocator_(NULL), reader_(NULL), tokeniser_(NULL), stack_(NULL), currentToken_(NULL),
    doc_(NULL), errors_(NULL) {
        
    }
    
    void TreeBuilder::initialiseParse(const StringRef& input, const StringRef& baseUri, ParseErrorList* errors, Allocator* allocator) {
        freeResources();
        
        CSOUP_ASSERT(input.size() > 0 && input.data() != NULL);
        CSOUP_ASSERT(baseUri.size() > 0 && baseUri.data() != NULL);
        
        // Don't destroy this
        errors_ = errors;
        
        if (allocator == NULL) {
            // if invoked didn't give an allocator, you should pass NULL to Document's construtor
            // and update allocator.
            // User can destroy the document using delete expression. And this should be the usual case.
            doc_ = new Document(baseUri, allocator);
            allocator = doc_->allocator();
        } else {
            // when user specify the allocator, you should initialize memory for Document from it.
            // User must invoke free of allocator in order to destroy Document.
            // User shouldn't use this style except the some extreme cases.
            doc_ = new (allocator->malloc_t<Document>()) Document(baseUri, allocator);
        }
        
        reader_ = new (allocator->malloc_t<CharacterReader>()) CharacterReader(input);
        tokeniser_ = new (allocator->malloc_t<Tokeniser>()) Tokeniser(reader_, errors, allocator);
        stack_ = new (allocator->malloc_t< internal::Vector<Element*> >()) internal::Vector<Element*>(4, allocator);
        baseUri_ = new (allocator->malloc_t< String>()) String(baseUri, allocator);
        allocator_ = allocator;
        currentToken_ = NULL;
    }
    
    TreeBuilder::~TreeBuilder() {
        freeResources();
    }
    
    void TreeBuilder::freeResources() {
        if (allocator_ == NULL) return ;
        
        allocator_->deconstructAndFree(reader_);            reader_         = NULL;
        allocator_->deconstructAndFree(tokeniser_);         tokeniser_      = NULL;
        allocator_->deconstructAndFree(stack_);             stack_          = NULL;
        allocator_->deconstructAndFree(baseUri_);            baseUri_        = NULL;
        if (currentToken_) {
            allocator_->deconstructAndFree(currentToken_);  currentToken_   = NULL;
        }
        
        // Don't destroy errors_! It's allocator outside treebuilder.
        
        allocator_  = NULL;
        doc_        = NULL;
        errors_     = NULL;
    }
    
    void TreeBuilder::runParser() {
        while (true) {
            Token* token = tokeniser_->read();
            process(token);
            
            bool isEnd = token->tokenType() == CSOUP_TOKEN_EOF;
            token->~Token();
            tokeniser_->allocator()->free(token);
            
            if (isEnd)
                break;
        }
    }
    
    void TreeBuilder::setTokeniserState(internal::TokeniserState *state) {
        CSOUP_ASSERT(state != NULL);
        tokeniser_->transition(state);
    }
    
    Element* TreeBuilder::currentElement() {
        return *stack_->back();
    }
    
    StringRef TreeBuilder::baseUri() const {
        return baseUri_ ? baseUri_->ref() : StringRef("");
    }
}
