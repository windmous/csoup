//
//  tokeniserstate.cpp
//  csoup
//
//  Created by mac on 12/13/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include "../util/common.h"
#include "../util/stringbuffer.h"
#include "tokeniserstate.h"
#include "tokeniser.h"
#include "token.h"
#include "characterreader.h"

using namespace csoup;

namespace csoup {
    using namespace internal;
    
    const int TokeniserState::nullChar_         = 0x0000;
    const int TokeniserState::replacementChar_  = Tokeniser::replacementChar_;
//    const CharType* TokeniserState::replacementStr_   = "\uFD\uFF";
    const int TokeniserState::eof_              = CharacterReader::EOF;
    
    void TokeniserState::handleDataEndTag(csoup::Tokeniser *t, csoup::CharacterReader *r, csoup::internal::TokeniserState *elseTransition) {
        if (isalpha(r->peek())) {
            StringBuffer name(t->allocator());
            appendUntilNotLetter(t, r, &name);
            t->appendDataBuffer(name.ref());
        
            name.tolower();
            t->appendTagName(name.ref());
            return ;
        }
        
        bool needsExitTransition = false;
        if (t->isAppropriateEndTagToken() && !r->empty()) {
            int c = r->next();
            switch (c) {
                case '\t':
                case '\n':
                case '\r':
                case '\f':
                case ' ':
                    t->transition(BeforeAttributeName::instance());
                    break;
                case '/':
                    t->transition(SelfClosingStartTag::instance());
                    break;
                case '>':
                    t->emitTagPending();
                    t->transition(Data::instance());
                    break;
                default:
                    t->appendDataBuffer(c);
                    needsExitTransition = true;
            }
        } else {
            needsExitTransition = true;
        }
        
        if (needsExitTransition) {
            t->emit(StringRef("</"));
            t->emit(t->bufferedData());
            t->transition(elseTransition);
        }
    }
    
    void TokeniserState::handleDataDoubleEscapeTag(csoup::Tokeniser *t, csoup::CharacterReader *r, csoup::internal::TokeniserState *primary, csoup::internal::TokeniserState *fallback) {
        if (std::isalpha(r->peek())) {
            StringBuffer name(t->allocator());
            appendUntilNotLetter(t, r, &name);
            
            t->emit(name.ref());
            
            name.tolower();
            t->appendDataBuffer(name.ref());
            
            return ;
        }
        
        int c = r->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
            case '/':
            case '>':
                if (internal::strEquals(t->bufferedData(), StringRef("script"))) {
                    t->transition(primary);
                } else {
                    t->transition(fallback);
                }
                t->emit(c);
                break;
            default:
                r->unconsume();
                t->transition(fallback);
        }
    }
    
    size_t TokeniserState::emitUntil(Tokeniser* t, CharacterReader* reader, CharType* endTerms, const size_t arrSize) {
        int c = reader->peek();
        size_t read = 0;
        
        while (c != CharacterReader::EOF) {
            for (size_t i = 0; i < arrSize; ++ i) {
                if (c == endTerms[i]) {
                    goto EMIT_UNTIL_OUTER;
                }
            }
            
            read ++;
            t->emit(c);
            c = reader->next();
        }
    EMIT_UNTIL_OUTER:
        return read;
    }
    
    size_t TokeniserState::lowercasedAppendUntil(Tokeniser* t, CharacterReader* reader,
                                                 StringBuffer* buffer, CharType* endTerms, const size_t arrSize) {
        int c = reader->peek();
        size_t read = 0;
        
        while (c != CharacterReader::EOF) {
            for (size_t i = 0; i < arrSize; ++ i) {
                if (c == endTerms[i]) {
                    goto LOWERCASED_APPEND_UNTIL;
                }
            }
            
            read ++;
            buffer->append(std::tolower(c));
            c = reader->next();
        }
        
    LOWERCASED_APPEND_UNTIL:
        return read;
    }
    
    size_t TokeniserState::lowercasedAppendUntilNotLetter(Tokeniser *t, CharacterReader *reader, StringBuffer *buffer) {
        int c = reader->peek();
        size_t read = 0;
        
        while (std::isalpha(c)) {
            read ++;
            buffer->append(std::tolower(c));
            c = reader->next();
        }
        
        return read;
    }
    
    size_t TokeniserState::appendUntilNotLetter(Tokeniser *t, CharacterReader *reader, StringBuffer *buffer) {
        int c = reader->peek();
        size_t read = 0;
        
        while (std::isalpha(c)) {
            read ++;
            buffer->append(c);
            c = reader->next();
        }
        
        return read;
    }
    
    size_t TokeniserState::appendUntil(csoup::Tokeniser *t, csoup::CharacterReader *reader, csoup::StringBuffer *buffer, CharType* endTerms, const size_t arrSize) {
        int c = reader->peek();
        size_t read = 0;
        
        while (c != CharacterReader::EOF) {
            for (size_t i = 0; i < arrSize; ++ i) {
                if (c == endTerms[i]) {
                    goto APPEND_UNTIL_OUTER;
                }
            }
            
            read ++;
            buffer->append(c);
            c = reader->next();
        }
        
    APPEND_UNTIL_OUTER:
        return read;
    }
    
     // in data state, gather characters until a character reference or tag is found
    void Data::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case '&':
                t->advanceTransition(CharacterReferenceInData::instance());
                break;
            case '<':
                t->advanceTransition(TagOpen::instance());
                break;
            case nullChar_:
                t->error(this); // NOT replacement character (oddly?)
                t->emit(reader->next());
                break;
            case eof_:
                t->emitEOF();
                break;
            default:
                CharType term[] = {'&', '<', nullChar_};
                emitUntil(t, reader, term, sizeof(term) / sizeof(*term));
                break;
        }
    }
    
    // from & in data
    void CharacterReferenceInData::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer buffer(t->allocator());
        bool ret = t->consumeCharacterReference(NULL, false, &buffer);
        
        if (!ret) {
            t->emit('&');
        } else {
            t->emit(buffer.ref());
        }
        
        t->transition(Data::instance());
    }
    
    // handles data in title, textarea etc
    void Rcdata::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case '&':
                t->advanceTransition(CharacterReferenceInData::instance());
                break;
            case '<':
                t->advanceTransition(RcdataLessthanSign::instance());
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            case eof_:
                t->emitEOF();
                break;
            default:
                CharType term[] = {'&', '<', nullChar_};
                emitUntil(t, reader, term, arrayLength(term));
                break;
        }
    }
    
    void CharacterReferenceInRcdata::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer buffer(t->allocator());
        bool ret = t->consumeCharacterReference(NULL, false, &buffer);
        
        if (!ret) {
            t->emit('&');
        } else {
            t->emit(buffer.ref());
        }
        
        t->transition(Rcdata::instance());
    }
    
    void RawText::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case '<':
                t->advanceTransition(RawtextLessthanSign::instance());
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            case eof_:
                t->emitEOF();
                break;
            default:
                CharType term[] = {'<', nullChar_};
                emitUntil(t, reader, term, arrayLength(term));
                break;
        }
    }
    
    void ScriptData::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case '<':
                t->advanceTransition(ScriptDataLessthanSign::instance());
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            case eof_:
                t->emitEOF();
                break;
                
            default:
                CharType term[] = {'<', nullChar_};
                emitUntil(t, reader, term, arrayLength(term));
                break;
        }
    }
    
    void PlainText::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            case eof_:
                t->emitEOF();
                break;
            default:
                CharType term[] = {nullChar_};
                emitUntil(t, reader, term, arrayLength(term));
                break;
        }
    }
    
    // from < in data
    void TagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->peek()) {
            case '!':
                t->advanceTransition(MarkupDeclarationOpen::instance());
                break;
            case '/':
                t->advanceTransition(EndTagOpen::instance());
                break;
            case '?':
                t->advanceTransition(BogusComment::instance());
                break;
            default:
                if (std::isalpha(reader->peek())) {
                    t->createTagPending(true);
                    t->transition(TagName::instance());
                } else {
                    t->error(this);
                    t->emit('<'); // char that got us here
                    t->transition(Data::instance());
                }
                break;
        }
    }
    
    void EndTagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->empty()) {
            t->eofError(this);
            t->emit("</");
            t->transition(Data::instance());
        } else if (std::isalpha(reader->peek())) {
            t->createTagPending(false);
            t->transition(TagName::instance());
        } else if (reader->matches('>')) {
            t->error(this);
            t->advanceTransition(Data::instance());
        } else {
            t->error(this);
            t->advanceTransition(BogusComment::instance());
        }
    }
    
    // from < or </ in data, will have start or end tag pending
    void TagName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        // previous TagOpen state did NOT consume, will have a letter char in current
        StringBuffer tagName(t->allocator());
        CharType terms[] = {'\t', '\n', '\r', '\f', ' ', '/', '>', nullChar_};
        lowercasedAppendUntil(t, reader, &tagName, terms, arrayLength(terms));
    
        t->appendTagName(tagName.ref());
        
        switch (reader->next()) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeAttributeName::instance());
                break;
            case '/':
                t->transition(SelfClosingStartTag::instance());
                break;
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                break;
            case nullChar_: // replacement
                t->appendTagName(replacementChar_);
                break;
            case eof_: // should emit pending tag?
                t->eofError(this);
                t->transition(Data::instance());
                // no default, as covered with above consumeToAny
        }
    }
    
    // This is not consistent with JSOUP!
    // We make this correspond with the spec;
    // from < in rcdata
    void RcdataLessthanSign::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matches('/')) {
            t->createTempBuffer();
            t->advanceTransition(RCDATAEndTagOpen::instance());
        } else {
            t->emit('<');
            t->transition(Rcdata::instance());
        }
    }
    
    void RCDATAEndTagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createTagPending(false);
            t->appendTagName(std::tolower(reader->peek()));
            t->appendDataBuffer(std::tolower(reader->peek()));
            t->advanceTransition(RCDATAEndTagName::instance());
        } else {
            t->emit("</");
            t->transition(Rcdata::instance());
        }
    }
    
    void RCDATAEndTagName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            StringBuffer name(t->allocator());
            appendUntilNotLetter(t, reader, &name);
        
            t->appendDataBuffer(name.ref());
            
            name.tolower();
            t->appendTagName(name.ref());
            
            return;
        }
        
#define RCDATA_END_TAG_NAME_ANYTHINGELSE \
    do { \
        t->emit(StringRef("</")); \
        t->emit(t->bufferedData()); \
        reader->unconsume(); \
        t->transition(Rcdata::instance()); \
    } while(0)
    
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                if (t->isAppropriateEndTagToken())
                    t->transition(BeforeAttributeName::instance());
                else
                    RCDATA_END_TAG_NAME_ANYTHINGELSE;
                break;
            case '/':
                if (t->isAppropriateEndTagToken())
                    t->transition(SelfClosingStartTag::instance());
                else
                    RCDATA_END_TAG_NAME_ANYTHINGELSE;
                break;
            case '>':
                if (t->isAppropriateEndTagToken()) {
                    t->emitTagPending();
                    t->transition(Data::instance());
                }
                else
                    RCDATA_END_TAG_NAME_ANYTHINGELSE;
                break;
            default:
                RCDATA_END_TAG_NAME_ANYTHINGELSE;
        }
    }
    
#undef RCDATA_END_TAG_NAME_ANYTHINGELSE
    
    void RawtextLessthanSign::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matches('/')) {
            t->createTempBuffer();
            t->advanceTransition(RawtextEndTagOpen::instance());
        } else {
            t->emit('<');
            t->transition(RawText::instance());
        }
    }
    
    void RawtextEndTagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createTagPending(false);
            t->transition(RawtextEndTagName::instance());
        } else {
            t->emit(StringRef("</"));
            t->transition(RawText::instance());
        }
    }
    
    void RawtextEndTagName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        handleDataEndTag(t, reader, RawText::instance());
    }
    
    void ScriptDataLessthanSign::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        switch (reader->next()) {
            case '/':
                t->createTempBuffer();
                t->transition(ScriptDataEndTagOpen::instance());
                break;
            case '!':
                t->emit("<!");
                t->transition(ScriptDataEscapeStart::instance());
                break;
            default:
                t->emit('<');
                reader->unconsume();
                t->transition(ScriptData::instance());
        }
    }
    
    void ScriptDataEndTagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createTagPending(false);
            t->transition(ScriptDataEndTagName::instance());
        } else {
            t->emit(StringRef("</"));
            t->transition(ScriptData::instance());
        }
    }
    
    void ScriptDataEndTagName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        handleDataEndTag(t, reader, ScriptData::instance());
    }
    
    void ScriptDataEscapeStart::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matches('-')) {
            t->emit('-');
            t->advanceTransition(ScriptDataEscapeStartDash::instance());
        } else {
            t->transition(ScriptData::instance());
        }
    }
    
    void ScriptDataEscapeStartDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matches('-')) {
            t->emit('-');
            t->advanceTransition(ScriptDataEscapedDashDash::instance());
        } else {
            t->transition(ScriptData::instance());
        }
    }
    
    void ScriptDataEscaped::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->empty()) {
            t->eofError(this);
            t->transition(Data::instance());
            return;
        }
        
        switch (reader->peek()) {
            case '-':
                t->emit('-');
                t->advanceTransition(ScriptDataEscapedDash::instance());
                break;
            case '<':
                t->advanceTransition(ScriptDataEscapedLessthanSign::instance());
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            default:
                CharType terms[] = {'-', '<', nullChar_};
                emitUntil(t, reader, terms, arrayLength(terms));
                break;
        }
    }
    
    void ScriptDataEscapedDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->empty()) {
            t->eofError(this);
            t->transition(Data::instance());
            
            return;
        }
        
        int c = reader->next();
        switch (c) {
            case '-':
                t->emit(c);
                t->transition(ScriptDataEscapedDashDash::instance());
                
                break;
            case '<':
                t->transition(ScriptDataEscapedLessthanSign::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->emit(replacementChar_);
                t->transition(ScriptDataEscaped::instance());
                
                break;
            default:
                t->emit(c);
                t->transition(ScriptDataEscaped::instance());
                
        }
    }
    void ScriptDataEscapedDashDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->empty()) {
            t->eofError(this);
            t->transition(Data::instance());
            
            return;
        }
        
        int c = reader->next();
        switch (c) {
            case '-':
                t->emit(c);
                break;
            case '<':
                t->transition(ScriptDataEscapedLessthanSign::instance());
                
                break;
            case '>':
                t->emit(c);
                t->transition(ScriptData::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->emit(replacementChar_);
                t->transition(ScriptDataEscaped::instance());
                
                break;
            default:
                t->emit(c);
                t->transition(ScriptDataEscaped::instance());
                
        }
    }
    void ScriptDataEscapedLessthanSign::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createTempBuffer();
            t->appendDataBuffer(std::tolower(reader->peek()));
            t->emit('<');
            t->emit(reader->peek());
            t->advanceTransition(ScriptDataDoubleEscapeStart::instance());
            
        } else if (reader->matches('/')) {
            t->createTempBuffer();
            t->advanceTransition(ScriptDataEscapedEndTagOpen::instance());
            
        } else {
            t->emit('<');
            t->transition(ScriptDataEscaped::instance());
            
        }
    }
    void ScriptDataEscapedEndTagOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createTagPending(false);
            t->appendTagName(std::tolower(reader->peek()));
            t->appendDataBuffer(reader->peek());
            t->advanceTransition(ScriptDataEscapedEndTagName::instance());
            
        } else {
            t->emit(StringRef("</"));
            t->transition(ScriptDataEscaped::instance());
            
        }
    }
    void ScriptDataEscapedEndTagName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        handleDataEndTag(t, reader, ScriptDataEscaped::instance());
    }
    void ScriptDataDoubleEscapeStart::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        handleDataDoubleEscapeTag(t, reader, ScriptDataDoubleEscaped::instance(), ScriptDataEscaped::instance());
    }
    void ScriptDataDoubleEscaped::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        char c = reader->peek();
        switch (c) {
            case '-':
                t->emit(c);
                t->advanceTransition(ScriptDataDoubleEscapedDash::instance());
                
                break;
            case '<':
                t->emit(c);
                t->advanceTransition(ScriptDataDoubleEscapedLessthanSign::instance());
                
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->emit(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            default:
                CharType term[] = {'-', '<', nullChar_};
                emitUntil(t, reader, term, arrayLength(term));

                break;
        }
    }
    void ScriptDataDoubleEscapedDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->emit(c);
                t->transition(ScriptDataDoubleEscapedDashDash::instance());
                
                break;
            case '<':
                t->emit(c);
                t->transition(ScriptDataDoubleEscapedLessthanSign::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->emit(replacementChar_);
                t->transition(ScriptDataDoubleEscaped::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            default:
                t->emit(c);
                t->transition(ScriptDataDoubleEscaped::instance());
                
        }
    }
    void ScriptDataDoubleEscapedDashDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->emit(c);
                break;
            case '<':
                t->emit(c);
                t->transition(ScriptDataDoubleEscapedLessthanSign::instance());
                
                break;
            case '>':
                t->emit(c);
                t->transition(ScriptData::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->emit(replacementChar_);
                t->transition(ScriptDataDoubleEscaped::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            default:
                t->emit(c);
                t->transition(ScriptDataDoubleEscaped::instance());
                
        }
    }
    void ScriptDataDoubleEscapedLessthanSign::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matches('/')) {
            t->emit('/');
            t->createTempBuffer();
            t->advanceTransition(ScriptDataDoubleEscapeEnd::instance());
            
        } else {
            t->transition(ScriptDataDoubleEscaped::instance());
            
        }
    }
    void ScriptDataDoubleEscapeEnd::read(csoup::Tokeniser *t, csoup::CharacterReader *r) {
        handleDataDoubleEscapeTag(t,r, ScriptDataEscaped::instance(), ScriptDataDoubleEscaped::instance());
    }
    // from tagname <xxx
    void BeforeAttributeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break; // ignore whitespace
            case '/':
                t->transition(SelfClosingStartTag::instance());
                
                break;
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->tagPending()->newAttribute();
                reader->unconsume();
                t->transition(AttributeName::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            case '"':
            case '\'':
            case '<':
            case '=':
                t->error(this);
                t->tagPending()->newAttribute();
                t->tagPending()->appendAttributeName(c);
                t->transition(AttributeName::instance());
                
                break;
            default: // A-Z, anything else
                t->tagPending()->newAttribute();
                reader->unconsume();
                t->transition(AttributeName::instance());
                
        }
    }
    // from before attribute name
    void AttributeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer name(t->allocator());
        CharType terms[] = {'\t', '\n', '\r', '\f', ' ', '/', '=', '>', nullChar_, '"', '\'', '<'};
        lowercasedAppendUntil(t, reader, &name, terms, arrayLength(terms));
        
        t->tagPending()->appendAttributeName(name.ref());
        
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(AfterAttributeName::instance());
                
                break;
            case '/':
                t->transition(SelfClosingStartTag::instance());
                
                break;
            case '=':
                t->transition(BeforeAttributeValue::instance());
                
                break;
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeName(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            case '"':
            case '\'':
            case '<':
                t->error(this);
                t->tagPending()->appendAttributeName(c);
                // no default, as covered in consumeToAny
        }
    }
    void AfterAttributeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                // ignore
                break;
            case '/':
                t->transition(SelfClosingStartTag::instance());
                
                break;
            case '=':
                t->transition(BeforeAttributeValue::instance());
                
                break;
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeName(replacementChar_);
                t->transition(AttributeName::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            case '"':
            case '\'':
            case '<':
                t->error(this);
                t->tagPending()->newAttribute();
                t->tagPending()->appendAttributeName(c);
                t->transition(AttributeName::instance());
                
                break;
            default: // A-Z, anything else
                t->tagPending()->newAttribute();
                reader->unconsume();
                t->transition(AttributeName::instance());
                
        }
    }
    void BeforeAttributeValue::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                // ignore
                break;
            case '"':
                t->transition(AttributeValue_doubleQuoted::instance());
                
                break;
            case '&':
                reader->unconsume();
                t->transition(AttributeValue_unquoted::instance());
                
                break;
            case '\'':
                t->transition(AttributeValue_singleQuoted::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeValue(replacementChar_);
                t->transition(AttributeValue_unquoted::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            case '>':
                t->error(this);
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case '<':
            case '=':
            case '`':
                t->error(this);
                t->tagPending()->appendAttributeValue(c);
                t->transition(AttributeValue_unquoted::instance());
                
                break;
            default:
                reader->unconsume();
                t->transition(AttributeValue_unquoted::instance());
                
        }
    }
    void AttributeValue_doubleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer value(t->allocator());
        CharType terms[] = {'"', '&', nullChar_};
        appendUntil(t, reader, &value, terms, arrayLength(terms));
        
        if (value.size() > 0)
            t->tagPending()->appendAttributeValue(value.ref());
        
        int c = reader->next();
        switch (c) {
            case '"':
                t->transition(AfterAttributeValue_quoted::instance());
                
                break;
            case '&': {
                int additionalAllowed = '"';
                StringBuffer buffer(t->allocator());
                bool ret = t->consumeCharacterReference(&additionalAllowed, true, &buffer);
                
                if (ret) {
                    t->tagPending()->appendAttributeValue(buffer.ref());
                } else {
                    t->tagPending()->appendAttributeValue('&');
                }
                break;
            }
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeValue(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
                // no default, handled in consume to any above
        }
    }
    void AttributeValue_singleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer value(t->allocator());
        CharType terms[] = {'\'', '&', nullChar_};
        appendUntil(t, reader, &value, terms, arrayLength(terms));
        
        if (value.size() > 0)
            t->tagPending()->appendAttributeValue(value.ref());
        
        int c = reader->next();
        switch (c) {
            case '\'':
                t->transition(AfterAttributeValue_quoted::instance());
                
                break;
            case '&': {
                int additionalAllowed = '\'';
                StringBuffer buffer(t->allocator());
                bool ret = t->consumeCharacterReference(&additionalAllowed, true, &buffer);
                
                if (ret)
                    t->tagPending()->appendAttributeValue(buffer.ref());
                else
                    t->tagPending()->appendAttributeValue('&');
                break;
            }
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeValue(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
                // no default, handled in consume to any above
        }
    }
    void AttributeValue_unquoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer value(t->allocator());
        CharType terms[] = {'\t', '\n', '\r', '\f', ' ', '&', '>', nullChar_, '"', '\'', '<', '=', '`'};
        appendUntil(t, reader, &value, terms, arrayLength(terms));
        
        if (value.size() > 0)
            t->tagPending()->appendAttributeValue(value.ref());
        
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeAttributeName::instance());
                
                break;
            case '&': {
                int additionalAllowed = '>';
                StringBuffer buffer(t->allocator());
                bool ret = t->consumeCharacterReference(&additionalAllowed, true, &buffer);
                
                if (ret)
                    t->tagPending()->appendAttributeValue(buffer.ref());
                else
                    t->tagPending()->appendAttributeValue('&');
                break;
            }
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->tagPending()->appendAttributeValue(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            case '"':
            case '\'':
            case '<':
            case '=':
            case '`':
                t->error(this);
                t->tagPending()->appendAttributeValue(c);
                break;
                // no default, handled in consume to any above
        }
        
    }
    
    // CharacterReferenceInAttributeValue state handled inline
    void AfterAttributeValue_quoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeAttributeName::instance());
                
                break;
            case '/':
                t->transition(SelfClosingStartTag::instance());
                
                break;
            case '>':
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                reader->unconsume();
                t->transition(BeforeAttributeName::instance());
                
        }
        
    }
    void SelfClosingStartTag::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '>':
                t->tagPending()->setSelfClosing();
                t->emitTagPending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->transition(BeforeAttributeName::instance());
                
        }
    }
    void BogusComment::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        // todo: handle bogus comment starting from eof_. when does that trigger?
        // rewind to capture character that lead us here
        reader->unconsume();
        CommentToken* comment = new (t->allocator()->malloc_t<CommentToken>()) CommentToken(t->allocator());
        comment->setBogus(true);
        
        StringBuffer value(t->allocator());
        CharType terms[] = {'>'};
        appendUntil(t, reader, &value, terms, arrayLength(terms));

        comment->append(value.ref());
        // todo: replace nullChar_ with replaceChar
        t->emit(comment);
        t->advanceTransition(Data::instance());
        
    }
    void MarkupDeclarationOpen::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->matchConsume("--")) {
            t->createCommentPending();
            t->transition(CommentStart::instance());
            
        } else if (reader->matchConsumeIgnoreCase("DOCTYPE")) {
            t->transition(Doctype::instance());
            
        } else if (reader->matchConsume("[CDATA[")) {
            // todo: should actually check current namepspace, and only non-html allows cdata. until namespace
            // is implemented properly, keep handling as cdata
            //} else if (!t->currentNodeInHtmlNS() && reader->matchConsume("[CDATA[")) {
            t->transition(CdataSection::instance());
            
        } else {
            t->error(this);
            t->advanceTransition(BogusComment::instance()); // advance so this character gets in bogus comment data's rewi::instance());
            
        }
    }
    void CommentStart::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->transition(CommentStartDash::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->commentPending()->append(replacementChar_);
                t->transition(Comment::instance());
                
                break;
            case '>':
                t->error(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default:
                t->commentPending()->append(c);
                t->transition(Comment::instance());
                
        }
    }
    void CommentStartDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->transition(CommentStartDash::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->commentPending()->append(replacementChar_);
                t->transition(Comment::instance());
                
                break;
            case '>':
                t->error(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default:
                t->commentPending()->append(c);
                t->transition(Comment::instance());
                
        }
    }
    void Comment::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        char c = reader->peek();
        switch (c) {
            case '-':
                t->advanceTransition(CommentEndDash::instance());
                
                break;
            case nullChar_:
                t->error(this);
                reader->advance();
                t->commentPending()->append(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default: {
                StringBuffer data(t->allocator());
                CharType term[] = {'-', nullChar_};
                appendUntil(t, reader, &data, term, arrayLength(term));
                t->commentPending()->append(data.ref());
            }
        }
    }
    void CommentEndDash::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->transition(CommentEnd::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->commentPending()->append('-');
                t->commentPending()->append(replacementChar_);
                t->transition(Comment::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default:
                t->commentPending()->append('-');
                t->commentPending()->append(c);
                t->transition(Comment::instance());
                
        }
    }
    void CommentEnd::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '>':
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->commentPending()->append(StringRef("--"));
                t->commentPending()->append(replacementChar_);
                t->transition(Comment::instance());
                
                break;
            case '!':
                t->error(this);
                t->transition(CommentEndBang::instance());
                
                break;
            case '-':
                t->error(this);
                t->commentPending()->append('-');
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->commentPending()->append(StringRef("--"));
                t->commentPending()->append(c);
                t->transition(Comment::instance());
                
        }
    }
    void CommentEndBang::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '-':
                t->commentPending()->append("--!");
                t->transition(CommentEndDash::instance());
                
                break;
            case '>':
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->commentPending()->append(StringRef("--!"));
                t->commentPending()->append(replacementChar_);
                t->transition(Comment::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->emitCommentPending();
                t->transition(Data::instance());
                
                break;
            default:
                t->commentPending()->append(StringRef("--!"));
                t->commentPending()->append(c);
                t->transition(Comment::instance());
                
        }
    }
    void Doctype::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeDoctypeName::instance());
                
                break;
            case eof_:
                t->eofError(this);
                // note: fall through to > case
            case '>': // catch invalid <!DOCTYPE>
                t->error(this);
                t->createDoctypePending();
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->transition(BeforeDoctypeName::instance());
                
        }
    }
    void BeforeDoctypeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            t->createDoctypePending();
            t->transition(DoctypeName::instance());
            
            return;
        }
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break; // ignore whitespace
            case nullChar_:
                t->error(this);
                t->createDoctypePending();
                t->doctypePending()->appendName(replacementChar_);
                t->transition(DoctypeName::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->createDoctypePending();
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->createDoctypePending();
                t->doctypePending()->appendName(c);
                t->transition(DoctypeName::instance());
                
        }
    }
    void DoctypeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (std::isalpha(reader->peek())) {
            StringBuffer name(t->allocator());
            lowercasedAppendUntilNotLetter(t, reader, &name);
            
            t->doctypePending()->appendName(name.ref());
            return;
        }
        int c = reader->next();
        switch (c) {
            case '>':
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(AfterDoctypeName::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->doctypePending()->appendName(replacementChar_);
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->doctypePending()->appendName(c);
        }
    }
    void AfterDoctypeName::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        if (reader->empty()) {
            t->eofError(this);
            t->doctypePending()->setForceQuirks(true);
            t->emitDoctypePending();
            t->transition(Data::instance());
            
            return;
        }
        
        if (reader->matchesAny('\t', '\n', '\r', '\f', ' '))
            reader->advance(); // ignore whitespace
        else if (reader->matches('>')) {
            t->emitDoctypePending();
            t->advanceTransition(Data::instance());
            
        } else if (reader->matchConsumeIgnoreCase("PUBLIC")) {
            t->transition(AfterDoctypePublicKeyword::instance());
            
        } else if (reader->matchConsumeIgnoreCase("SYSTEM")) {
            t->transition(AfterDoctypeSystemKeyword::instance());
            
        } else {
            t->error(this);
            t->doctypePending()->setForceQuirks(true);
            t->advanceTransition(BogusDoctype::instance());
            
        }
        
    }
    void AfterDoctypePublicKeyword::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeDoctypePublicIdentifier::instance());
                
                break;
            case '"':
                t->error(this);
                // set public id to empty string
                t->transition(DoctypePublicIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                t->error(this);
                // set public id to empty string
                t->transition(DoctypePublicIdentifier_singleQuoted::instance());
                
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->transition(BogusDoctype::instance());
                
        }
    }
    void BeforeDoctypePublicIdentifier::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break;
            case '"':
                // set public id to empty string
                t->transition(DoctypePublicIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                // set public id to empty string
                t->transition(DoctypePublicIdentifier_singleQuoted::instance());
                
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->transition(BogusDoctype::instance());
                
        }
    }
    void DoctypePublicIdentifier_doubleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '"':
                t->transition(AfterDoctypePublicIdentifier::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->doctypePending()->appendPublicIdentifier(replacementChar_);
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->doctypePending()->appendPublicIdentifier(c);
        }
    }
    void DoctypePublicIdentifier_singleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\'':
                t->transition(AfterDoctypePublicIdentifier::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->doctypePending()->appendPublicIdentifier(replacementChar_);
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->doctypePending()->appendPublicIdentifier(c);
        }
    }
    void AfterDoctypePublicIdentifier::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BetweenDoctypePublicAndSystemIdentifiers::instance());
                
                break;
            case '>':
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case '"':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_singleQuoted::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->transition(BogusDoctype::instance());
                
        }
    }
    void BetweenDoctypePublicAndSystemIdentifiers::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break;
            case '>':
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case '"':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_singleQuoted::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->transition(BogusDoctype::instance());
                
        }
    }
    void AfterDoctypeSystemKeyword::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                t->transition(BeforeDoctypeSystemIdentifier::instance());
                
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case '"':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                t->error(this);
                // system id empty
                t->transition(DoctypeSystemIdentifier_singleQuoted::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
        }
    }
    void BeforeDoctypeSystemIdentifier::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break;
            case '"':
                // set system id to empty string
                t->transition(DoctypeSystemIdentifier_doubleQuoted::instance());
                
                break;
            case '\'':
                // set public id to empty string
                t->transition(DoctypeSystemIdentifier_singleQuoted::instance());
                
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->transition(BogusDoctype::instance());
                
        }
    }
    void DoctypeSystemIdentifier_doubleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '"':
                t->transition(AfterDoctypeSystemIdentifier::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->doctypePending()->appendSystemIdentifier(replacementChar_);
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->doctypePending()->appendSystemIdentifier(c);
        }
    }
    void DoctypeSystemIdentifier_singleQuoted::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\'':
                t->transition(AfterDoctypeSystemIdentifier::instance());
                
                break;
            case nullChar_:
                t->error(this);
                t->doctypePending()->appendSystemIdentifier(replacementChar_);
                break;
            case '>':
                t->error(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->doctypePending()->appendSystemIdentifier(c);
        }
    }
    void AfterDoctypeSystemIdentifier::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ' ':
                break;
            case '>':
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->eofError(this);
                t->doctypePending()->setForceQuirks(true);
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                t->error(this);
                t->transition(BogusDoctype::instance());
                
                // NOT force quirks
        }
    }
    void BogusDoctype::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        int c = reader->next();
        switch (c) {
            case '>':
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            case eof_:
                t->emitDoctypePending();
                t->transition(Data::instance());
                
                break;
            default:
                // ignore char
                break;
        }
    }
    void CdataSection::read(csoup::Tokeniser *t, csoup::CharacterReader *reader) {
        StringBuffer data(t->allocator());
        reader->consumeTo("]]>", &data);
        t->emit(data.ref());
        reader->matchConsume("]]>");
        t->transition(Data::instance());
        
    }
}

