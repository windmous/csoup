//
//  characterreader.cpp
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include <iostream>

#include "characterreader.h"
#include "stringbuffer.h"

namespace {
    const int kUtf8ReplacementChar = 0xFFFD;
    #define UTF8_ACCEPT 0
    #define UTF8_REJECT 12

    const uint8_t utf8d[] = {
        // The first part of the table maps bytes to character classes that
        // to reduce the size of the transition table and create bitmasks.
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
        
        // The second part is a transition table that maps a combination
        // of a state of the automaton and a character class to a state.
        0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12, 
    };
    
    
    uint32_t inline decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
        uint32_t type = utf8d[byte];
        
        *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3fu) | (*codep << 6) :
        (0xff >> type) & (byte);
        
        *state = utf8d[256 + *state + type];
        return *state;
    }
    
    bool isInvalidUTF8CodePoint(int c) {
        return (c >= 0x1 && c <= 0x8) || c == 0xB || (c >= 0xE && c <= 0x1F) ||
        (c >= 0x7F && c <= 0x9F) || (c >= 0xFDD0 && c <= 0xFDEF) ||
        ((c & 0xFFFF) == 0xFFFE) || ((c & 0xFFFF) == 0xFFFF);
    }
}

namespace csoup {
    // we uuse
    CSOUP_STATIC_ASSERT(sizeof(CharType) == sizeof(char));
    
    void CharacterReader::readChar() {
        if (cur_ >= end_) {
            // No input left to consume; emit an EOF and set width = 0.
            current_ = -1;
            width_ = 0;
            return;
        }
        
        uint32_t code_point = 0;
        uint32_t state = UTF8_ACCEPT;
        for (const CharType* c = cur_; c < end_; ++ c) {
            decode(&state, &code_point, (uint32_t) (unsigned char) (*c));
            if (state == UTF8_ACCEPT) {
                width_ = c - cur_ + 1;
                // This is the special handling for carriage returns that is mandated by the
                // HTML5 spec.  Since we're looking for particular 7-bit literal characters,
                // we operate in terms of chars and only need a check for iter overrun,
                // instead of having to read in a full next code point.
                // http://www.whatwg.org/specs/web-apps/current-work/multipage/parsing.html#preprocessing-the-input-stream
                if (code_point == '\r') {
                    CSOUP_ASSERT(width_ == 1);
                    const char* next = c + 1;
                    if (next < end_ && *next == '\n') {
                        // Advance the iter, as if the carriage return didn't exist.
                        ++cur_;
                        // Preserve the true offset, since other tools that look at it may be
                        // unaware of HTML5's rules for converting \r into \n.
                        //++iter->_pos.offset;
                    }
                    code_point = '\n';
                }
                if (isInvalidUTF8CodePoint(code_point)) {
                    //add_error(iter, GUMBO_ERR_UTF8_INVALID);
                    //CSOUP_ASSERT()
                    std::cout << "UTF8 decoding error" << std::endl;
                    code_point = kUtf8ReplacementChar;
                }
                current_ = code_point;
                return;
            } else if (state == UTF8_REJECT) {
                // We don't want to consume the invalid continuation byte of a multi-byte
                // run, but we do want to skip past an invalid first byte.
                width_ = c - cur_ + (c == cur_);
                current_ = kUtf8ReplacementChar;
                std::cout << "UTF8 decoding error" << std::endl;
                //add_error(iter, GUMBO_ERR_UTF8_INVALID);
                return;
            }
        }
        // If we got here without exiting early, then we've reached the end of the iterator.
        // Add an error for truncated input, set the width to consume the rest of the
        // iterator, and emit a replacement character.  The next time we enter this method,
        // it will detect that there's no input to consume and
        current_ = kUtf8ReplacementChar;
        width_ = end_ - start_;
        std::cout << "UTF8 decoding error: TRUNCATED" << std::endl;
        //add_error(iter, GUMBO_ERR_UTF8_TRUNCATED);
    }
    
    void CharacterReader::consumeTo(const csoup::StringRef &term, csoup::StringBuffer *output) {
        size_t offset = nextIndexOf(term);
        if (start_ + offset < end_) {
            StringRef data(cur_, offset - pos());
            output->appendString(data);
            cur_ = start_ + offset;
            
            readChar();
        } else {
            consumeToEnd(output);
        }
    }
    
    void CharacterReader::consumeToEnd(csoup::StringBuffer *output) {
        while (!empty()) {
            output->append(next());
        }
    }
    
    // need to be re-implemented
    size_t CharacterReader::nextIndexOf(int c) {
        CSOUP_ASSERT(c <= 127 && c >= 0);
        
        const CharType* p = cur_;
        
        while (p < end_ && *p != c) {
            p ++;
        }
        
        return p - start_;
    }
    
    size_t CharacterReader::nextIndexOf(const csoup::StringRef &seq) {
        CSOUP_ASSERT(seq.size() > 0);
        
        if (seq.size() == 1) {
            return nextIndexOf(seq.at(0));
        }
        
        int startCode = seq.at(0);
        for (const CharType* p = cur_; p < end_; ++ p) {
            if (startCode != *p) {
                while (p < end_ && startCode != *p) ++ p;
            }
            
            const CharType* p2 = p + 1;
            const CharType* last = (p2 + seq.size()) - 1;
            if (p2 < end_ && last <= end_) {
                for (size_t i = 1; p2 < last && *p2 == seq.at(i); ++ p2, ++ i);
                if (p2 == end_) return p - start_;
            }
        }
        
        return end_ - start_;
    }
}