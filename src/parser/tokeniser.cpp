//
//  tokeniser.cpp
//  csoup
//
//  Created by mac on 12/13/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include <cctype>

#include "../nodes/entities.h"
#include "characterreader.h"
#include "tokeniserstate.h"
#include "tokeniser.h"
#include "token.h"
#include "util/common.h"
#include "util/stringref.h"
#include "util/stringbuffer.h"
#include "util/csoup_string.h"
#include "util/allocators.h"


namespace csoup {
    Tokeniser::Tokeniser(CharacterReader* reader, ParseErrorList* errorList, Allocator* allocator) :
        allocator_(allocator), reader_(reader), errors_(errorList),
        state_(internal::Data::instance()), emitPending_(NULL), isEmitPending_(false),
        charBuffer_(NULL), dataBuffer_(NULL), tagPending_(NULL), doctypePending_(NULL),
        commentPending_(NULL), lastStartTag_(NULL), selfClosingFlagAcknowledged(true) {
        
        CSOUP_ASSERT(allocator != NULL);
        CSOUP_ASSERT(reader != NULL);
        CSOUP_ASSERT(errorList != NULL);
            
        charBuffer_ = new (allocator->malloc_t<StringBuffer>()) StringBuffer(allocator);
    }
    
    Tokeniser::~Tokeniser() {
        destroy(&charBuffer_);
        // you must destroying other memebers
    }
    
    Token* Tokeniser::read() {
        if (!selfClosingFlagAcknowledged) {
            error(StringRef("Self closing flag not acknowledged"));
            selfClosingFlagAcknowledged = true;
        }
        
        while (isEmitPending_) {
            state_->read(this, reader_);
        }
        
        Token* ret;
        if (charBuffer_->size() > 0) {
            ret = new (allocator_->malloc_t<CharacterToken>()) CharacterToken(charBuffer_->ref(), allocator_);
            charBuffer_->clear();
        } else {
            isEmitPending_ = false;
            ret = emitPending_;
            
            // note that JSOUP didn't do this; I just guess the implementation
            emitPending_ = NULL;
        }
        
        return ret;
    }
    
    void Tokeniser::emit(Token* token) {
        // Need to be reconsidered;
//        Token* candidates[] = {tagPending_, doctypePending_, commentPending_, lastStartTag_, emitPending_};
//        bool found = false;
//        for (size_t i = 0; i < sizeof(candidates) / sizeof(*candidates); ++ i) {
//            if (token == candidates[i]) {
//                found = true;
//                break;
//            }
//        }
        
        CSOUP_ASSERT(emitPending_ == NULL);
        emitPending_ = token;
    }
    
    void Tokeniser::emit(const StringRef& str) {
        charBuffer_->appendString(str);
    }
    
    void Tokeniser::emit(int c) {
        charBuffer_->append(c);
    }
    
    void Tokeniser::emitEOF() {
        EOFToken* eof = new (allocator_->malloc_t<EOFToken>()) EOFToken();
        emit(eof);
    }
    
    void Tokeniser::transition(internal::TokeniserState* state) {
        state_ = state;
    }
    
    StringRef Tokeniser::bufferedData() {
        return dataBuffer_ == NULL ? StringRef("") : dataBuffer_->ref();
    }
    
    void Tokeniser::appendBufferedDataToEmitPendingString() {
        if (dataBuffer_ != NULL) {
            charBuffer_->appendString(dataBuffer_->ref());
        }
    }
    
    void Tokeniser::appendTagName(const csoup::StringRef &append) {
        CSOUP_ASSERT(tagPending_ != NULL);
        tagPending_->appendTagName(append);
    }
    
    void Tokeniser::appendTagName(const int c) {
        CSOUP_ASSERT(tagPending_ != NULL);
        tagPending_->appendTagName(c);
    }
    
    void Tokeniser::appendDataBuffer(const csoup::StringRef &append) {
        CSOUP_ASSERT(dataBuffer_ != NULL);
        dataBuffer_->appendString(append);
    }
    
    void Tokeniser::appendDataBuffer(const int c) {
        CSOUP_ASSERT(dataBuffer_ != NULL);
        dataBuffer_->append(c);
    }
    
    void Tokeniser::advanceTransition(internal::TokeniserState* state) {
        reader_->advance();
        state_ = state;
    }
    
    bool Tokeniser::consumeCharacterReference(int* additionalAllowedCharacter, bool inAttribute, StringBuffer* output) {
        CSOUP_ASSERT(output != NULL);
        
        if (reader_->empty()) {
            return false;
        }
        
        if (additionalAllowedCharacter != NULL && *additionalAllowedCharacter == reader_->peek()) {
            return false;
        }
        
        int badStart[] = {'\t', '\n', '\r', '\f', ' ', '<', '&'};
        if (reader_->matchesAny(badStart, sizeof(badStart) / sizeof(*badStart))) {
            return false;
        }
        
        StringBuffer buffer(allocator_);
        reader_->mark();
        
        if (reader_->matchConsume('#')) {
            bool isHexMode = reader_->matchConsumeIgnoreCase('X');
            if (isHexMode) {
                readHexSequence(&buffer);
            } else {
                readDigitSequence(&buffer);
            }
            
            if (buffer.size() == 0) {
                characterReferenceError(StringRef("numeric reference with no numerals"));
                reader_->rewindToMark();
                return false;
            }
            
            if (!reader_->matchConsume(';')) {
                characterReferenceError(StringRef("missing semicolon"));
            }
            
            int64_t charval = -1;
            
            int base = isHexMode ? 16 : 10;
            for (size_t i = 0; i < buffer.size(); ++ i) {
                charval = charval * base + (buffer.data()[i] - '0');
                
                if (charval > (unsigned int)0xFFFFFFFF) {
                    characterReferenceError(StringRef("value is overflow"));
                    charval = -1;
                    break;
                }
            }
            
            if (charval == -1 || (charval >= 0xD800 && charval <= 0xDFFF) || charval > 0x10FFFF) {
                characterReferenceError("character outside of valid range");
                output->append(replacementChar_);
            } else {
                // TODO: We must check if charval is an valid utf8 codepoint
                output->append(static_cast<int>(charval));
            }
        } else {
            readReferenceName(&buffer);
            buffer.append('\0');
            
            bool looksLegit = reader_->matches(';');
            bool found = (Entities::isBaseNamedEntity(buffer.data()) ||
                          (Entities::isNamedEntity(buffer.data()) && looksLegit));
            
            if (!found) {
                reader_->rewindToMark();
                if (looksLegit) {
                    characterReferenceError("invalid named referenece");
                }
                
                return false;
            }
            
            int c = reader_->peek();
            if (inAttribute && (std::isalpha(c) || std::isdigit(c) || c == '-' || c == '-' || c == '_')) {
                reader_->rewindToMark();
                return false;
            }
            
            if (!reader_->matchConsume(';')) {
                characterReferenceError("missing semicolon"); // missing semi
            }
            output->append(Entities::getCharacterByName(buffer.data()));
        }
        
        return true;
    }
    
    TagToken* Tokeniser::createTagPending(bool start) {
        CSOUP_ASSERT(tagPending_ == NULL);
        if (start) {
            tagPending_ = new (allocator_->malloc_t<StartTagToken>()) StartTagToken(allocator_);
        } else {
            tagPending_ = new (allocator_->malloc_t<EndTagToken>()) EndTagToken(allocator_);
        }
            
        return tagPending_;
    }
    
    // You may have to set tagPending_ to be NULL
    void Tokeniser::emitTagPending() {
        tagPending_->finaliseTag();
        emit(tagPending_);
        tagPending_ = NULL;
    }
    
    void Tokeniser::createCommentPending() {
        commentPending_ = new (allocator_->malloc_t<CommentToken>()) CommentToken(allocator_);
    }
    
    void Tokeniser::emitCommentPending() {
        emit(commentPending_);
        commentPending_ = NULL;
    }
    
    void Tokeniser::createDoctypePending() {
        doctypePending_ = new (allocator_->malloc_t<DoctypeToken>()) DoctypeToken(allocator_);
    }
    
    void Tokeniser::emitDoctypePending() {
        emit(doctypePending_);
        doctypePending_ = NULL;
    }
    
    void Tokeniser::createTempBuffer() {
        if (dataBuffer_) {
            dataBuffer_->~StringBuffer();
        } else {
            dataBuffer_ = allocator_->malloc_t<StringBuffer>();
        }
        
        new (dataBuffer_) StringBuffer(allocator_);
    }
    bool Tokeniser::isAppropriateEndTagToken() {
        if (lastStartTag_ == NULL) return false;
        return internal::strEqualsIgnoreCase(tagPending_->tagName(), lastStartTag_->tagName());
    }
    
    StringRef Tokeniser::appropriateEndTagName() {
        if (lastStartTag_ == NULL) {
            return StringRef("");
        }
        
        return lastStartTag_->tagName();
    }
    
    void Tokeniser::error(internal::TokeniserState* state) {
        //errors_->
    }
    void Tokeniser::eofError(internal::TokeniserState* state) {
        
    }
    
    void Tokeniser::characterReferenceError(const StringRef& message) {
        
    }
    void Tokeniser::error(const StringRef& errorMsg) {
        
    }
    
    void Tokeniser::readHexSequence(StringBuffer *buffer) {
        int c = reader_->peek();
        while (std::isxdigit(c)) {
            buffer->append(c);
            c = reader_->next();
        }
    }
    
    void Tokeniser::readDigitSequence(csoup::StringBuffer *buffer) {
        int c = reader_->peek();
        while (std::isdigit(c)) {
            buffer->append(c);
            c = reader_->next();
        }
    }
    
    void Tokeniser::readReferenceName(csoup::StringBuffer *output) {
        int c = reader_->peek();
        while (std::isalpha(c)) {
            output->append(c);
            c = reader_->next();
        }
        
        while (std::isdigit(c)) {
            output->append(c);
            c = reader_->next();
        }
    }
}