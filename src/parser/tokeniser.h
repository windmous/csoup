//
//  tokeniser.h
//

//
//  Created by mac on 12/13/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_TOKENISER_H_
#define CSOUP_TOKENISER_H_

namespace csoup {
    // Some class declarations
    namespace internal {
        class TokeniserState;
    }
    
    class CharacterReader;
    class ParseErrorList;
    class Token;
    class Allocator;
    class StringBuffer;
    class Tag;
    class TagToken;
    class StartTagToken;
    class DoctypeToken;
    class CommentToken;
    class StringRef;
    
    class Tokeniser {
    public:
        Tokeniser(CharacterReader* reader, ParseErrorList* errorList, Allocator* allocator);
        ~Tokeniser();
        
        // The invoker must destroy the returned token!!
        Token* read();
        
        
        // this is not consistent with our philosogy
        // need to be reconsidered
        void emit(Token* token);
        
        void emit(const StringRef& str);
        void emit(int c);
        
        void emitEOF();
        
        // user can't deconstruct state!
        internal::TokeniserState& state() {
            return *state_;
        }
        
        // the two methods below is not elegant.
        void transition(internal::TokeniserState* state);
        void advanceTransition(internal::TokeniserState* state);
        
        void setAcknowledgeSelfClosingFlag() {
            selfClosingFlagAcknowledged = true;
        }
        
        TagToken* createTagPending(bool start);
        void emitTagPending();
        
        void createCommentPending();
        void emitCommentPending();
        CommentToken* commentPending() {
            return commentPending_;
        }
        
        void createDoctypePending();
        void emitDoctypePending();
        DoctypeToken* doctypePending() {
            return doctypePending_;
        }
        
        void createTempBuffer();
        bool isAppropriateEndTagToken();
        
        StringRef appropriateEndTagName();
        
        void error(internal::TokeniserState* state);
        void eofError(internal::TokeniserState* state);
        
        void characterReferenceError(const StringRef& message);
        void error(const StringRef& errorMsg);
        
        bool currentNodeInHtmlNS() {
            return true;
        }
        
        Allocator* allocator() {
            return allocator_;
        }
        
        StringRef bufferedData();
        void appendBufferedDataToEmitPendingString();
        
        void appendTagName(const StringRef& append);
        void appendTagName(const int c);
        
        void appendDataBuffer(const StringRef& append);
        void appendDataBuffer(const int c);
        
        // not very perfectly complemented
        bool consumeCharacterReference(int* additionalAllowedCharacter, bool inAttribute, StringBuffer* buffer);
        
        TagToken* tagPending() {
            return tagPending_;
        }
        
        static const unsigned int replacementChar_ = 0xFFFD;
    private:
        
        void readHexSequence(StringBuffer* output);
        void readDigitSequence(StringBuffer* output);
        void readReferenceName(StringBuffer* output);

        Allocator* allocator_;
        CharacterReader* reader_;
        ParseErrorList* errors_;
        internal::TokeniserState* state_;
        Token* emitPending_;
        bool isEmitPending_;
        StringBuffer* charBuffer_;
        StringBuffer* dataBuffer_;
        
        TagToken* tagPending_;
        DoctypeToken* doctypePending_;
        CommentToken* commentPending_;
        StartTagToken* lastStartTag_;
        
        bool selfClosingFlagAcknowledged;
    };
}

#endif // CSOUP_TOKENISER_H_
