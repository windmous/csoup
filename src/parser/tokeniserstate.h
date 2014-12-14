//
//  tokeniserstate.h
//  csoup
//
//  Created by mac on 12/13/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_TOKENISER_STATE_H_
#define CSOUP_TOKENISER_STATE_H_

namespace csoup {
    class Tokeniser;
    class CharacterReader;
    class Allocator;
    class StringBuffer;
    
    namespace internal {
    
        class TokeniserState {
        public:
            virtual void read(Tokeniser* t, CharacterReader* reader) = 0;
            
        protected:
            static void handleDataEndTag(Tokeniser* t, CharacterReader* r, TokeniserState* elseTransition);
            
            static void handleDataDoubleEscapeTag(Tokeniser* t, CharacterReader* r, TokeniserState* primary, TokeniserState* fallback);
            
            static size_t emitUntil(Tokeniser* t, CharacterReader* reader, CharType* arr,  const size_t n);
            
            static size_t lowercasedAppendUntil(Tokeniser* t, CharacterReader* reader, StringBuffer* buffer, CharType* terms, const size_t n);
            
            static size_t lowercasedAppendUntilNotLetter(Tokeniser* t, CharacterReader*    reader, StringBuffer* buffer);
            
            static size_t appendUntilNotLetter(Tokeniser *t, CharacterReader *reader, StringBuffer *buffer);
            
            static size_t appendUntil(Tokeniser* t, CharacterReader* reader, StringBuffer* buffer, CharType* terms, const size_t n);
            
            static const int nullChar_;
            static const int replacementChar_;
            static const CharType* replacementStr_;
            static const int eof_;
        };
        
#define CSOUP_REGISTER_TOKENISER_STATE(StateName) \
    class StateName : public TokeniserState { \
    public: \
        void read(Tokeniser* t, CharacterReader* reader); \
        static StateName* instance() { \
            static StateName globalInstance; \
            return &globalInstance; \
        }\
    };
        
        CSOUP_REGISTER_TOKENISER_STATE(Data)
        CSOUP_REGISTER_TOKENISER_STATE(CharacterReferenceInData)
        CSOUP_REGISTER_TOKENISER_STATE(Rcdata)
        CSOUP_REGISTER_TOKENISER_STATE(CharacterReferenceInRcdata)
        CSOUP_REGISTER_TOKENISER_STATE(RawText)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptData)
        CSOUP_REGISTER_TOKENISER_STATE(PlainText);
        CSOUP_REGISTER_TOKENISER_STATE(TagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(EndTagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(TagName)
        CSOUP_REGISTER_TOKENISER_STATE(RcdataLessthanSign)
        CSOUP_REGISTER_TOKENISER_STATE(RCDATAEndTagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(RCDATAEndTagName)
        CSOUP_REGISTER_TOKENISER_STATE(RawtextLessthanSign)
        CSOUP_REGISTER_TOKENISER_STATE(RawtextEndTagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(RawtextEndTagName)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataLessthanSign)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEndTagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEndTagName)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapeStart)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapeStartDash)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscaped)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapedDash)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapedDashDash)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapedLessthanSign)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapedEndTagOpen)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataEscapedEndTagName)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscapeStart)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscaped)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscapedDash)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscapedDashDash)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscapedLessthanSign)
        CSOUP_REGISTER_TOKENISER_STATE(ScriptDataDoubleEscapeEnd)
        CSOUP_REGISTER_TOKENISER_STATE(BeforeAttributeName)
        CSOUP_REGISTER_TOKENISER_STATE(AttributeName)
        CSOUP_REGISTER_TOKENISER_STATE(AfterAttributeName)
        CSOUP_REGISTER_TOKENISER_STATE(BeforeAttributeValue)
        CSOUP_REGISTER_TOKENISER_STATE(AttributeValue_doubleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(AttributeValue_singleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(AttributeValue_unquoted)
        CSOUP_REGISTER_TOKENISER_STATE(AfterAttributeValue_quoted)
        CSOUP_REGISTER_TOKENISER_STATE(SelfClosingStartTag)
        CSOUP_REGISTER_TOKENISER_STATE(BogusComment)
        CSOUP_REGISTER_TOKENISER_STATE(MarkupDeclarationOpen)
        CSOUP_REGISTER_TOKENISER_STATE(CommentStart)
        CSOUP_REGISTER_TOKENISER_STATE(CommentStartDash)
        CSOUP_REGISTER_TOKENISER_STATE(Comment)
        CSOUP_REGISTER_TOKENISER_STATE(CommentEndDash)
        CSOUP_REGISTER_TOKENISER_STATE(CommentEnd)
        CSOUP_REGISTER_TOKENISER_STATE(CommentEndBang)
        CSOUP_REGISTER_TOKENISER_STATE(Doctype)
        CSOUP_REGISTER_TOKENISER_STATE(BeforeDoctypeName)
        CSOUP_REGISTER_TOKENISER_STATE(DoctypeName)
        CSOUP_REGISTER_TOKENISER_STATE(AfterDoctypeName)
        CSOUP_REGISTER_TOKENISER_STATE(AfterDoctypePublicKeyword)
        CSOUP_REGISTER_TOKENISER_STATE(BeforeDoctypePublicIdentifier)
        CSOUP_REGISTER_TOKENISER_STATE(DoctypePublicIdentifier_doubleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(DoctypePublicIdentifier_singleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(AfterDoctypePublicIdentifier)
        CSOUP_REGISTER_TOKENISER_STATE(BetweenDoctypePublicAndSystemIdentifiers)
        CSOUP_REGISTER_TOKENISER_STATE(AfterDoctypeSystemKeyword)
        CSOUP_REGISTER_TOKENISER_STATE(BeforeDoctypeSystemIdentifier)
        CSOUP_REGISTER_TOKENISER_STATE(DoctypeSystemIdentifier_doubleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(DoctypeSystemIdentifier_singleQuoted)
        CSOUP_REGISTER_TOKENISER_STATE(AfterDoctypeSystemIdentifier)
        CSOUP_REGISTER_TOKENISER_STATE(BogusDoctype)
        CSOUP_REGISTER_TOKENISER_STATE(CdataSection)

#undef CSOUP_REGISTER_TOKENISER_STATE
    }
}

#endif // CSOUP_TOKENISER_STATE_H_
