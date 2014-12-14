//
//  characterreader.h
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_CHARACTER_READER_H_
#define CSOUP_CHARACTER_READER_H_

#include <cctype>
#include "../util/common.h"
#include "../util/stringref.h"

namespace csoup {
    class StringBuffer;
    
    class CharacterReader {
    public:
        CharacterReader(const StringRef& input): start_(input.data()),
                                                 cur_(input.data()),
                                                 mark_(input.data()),
                                                 end_(input.data() + input.size()),
                                                 current_(0),
                                                 width_(0)
        {
            CSOUP_ASSERT(start_ != NULL);
        }
        
        size_t pos() const {
            return cur_ - start_;
        }
        
        bool empty() const {
            return start_ >= end_;
        }
        
        void unconsume() {
            CSOUP_ASSERT(width_ > 0);
            cur_ -= width_;
            width_ = 0;
        }
        
        void advance() {
            cur_ += width_;
            readChar();
        }
        
        int next() {
            int ret = current_;
            cur_ += width_;
            readChar();
            
            return ret;
        }
        
        int peek() const {
            return current_;
        }
        
        void mark() {
            mark_ = cur_;
        }
        
        void rewindToMark() {
            cur_ = mark_;
        }
        
        StringRef consumeAsStringRef() {
            StringRef ret(cur_, width_);
            cur_ += width_;
            readChar();
            return ret;
        }
        
        bool matches(int c) {
            return peek() == c;
        }
        
        bool matches(const StringRef& seq) {
            size_t scanLength = seq.size();
            if (scanLength > end_ - cur_)
                return false;
            
            for (size_t offset = 0; offset < scanLength; offset++) {
                if (seq.at(offset) != cur_[offset])
                    return false;
            }
            
            return true;
        }
        
        bool matchesIgnoreCase(int c) {
            return std::tolower(peek()) == std::tolower(c);
        }
        
        bool matchesIgnoreCase(const StringRef& seq) {
            size_t scanLength = seq.size();
            if (scanLength > end_ - cur_)
                return false;
            
            for (size_t offset = 0; offset < scanLength; offset++) {
                if (std::tolower(seq.at(offset)) != std::tolower(cur_[offset]))
                    return false;
            }
            
            return true;
        }
        
        bool matchConsume(int c) {
            if (matches(c)) {
                advance();
                return true;
            } else {
                return false;
            }
        }
        
        bool matchConsumeIgnoreCase(int c) {
            if (matchesIgnoreCase(c)) {
                advance();
                return true;
            } else {
                return false;
            }
        }
        
        bool matchesAny(int* seq, size_t cnt) {
            if (empty()) return false;
            int c = peek();
            
            for (size_t i = 0; i < cnt; ++ i) {
                if (c == seq[cnt]) {
                    return true;
                }
            }
            
            return false;
        }
        
        void consumeTo(const StringRef& term, StringBuffer* output);
        
        bool matchConsume(const StringRef& str) {
            if (matches(str)) {
                cur_ += str.size();
                readChar();
                return true;
            } else {
                return false;
            }
        }
        
        bool matchConsumeIgnoreCase(const StringRef& str) {
            if (matchesIgnoreCase(str)) {
                cur_ += str.size();
                readChar();
                return true;
            } else {
                return false;
            }
        }
        
        bool matchesAny(int c) {
            return peek() == c;
        }
        
        bool matchesAny(int c1, int c2) {
            return peek() == c1 || peek() == c2;
        }
        
        bool matchesAny(int c1, int c2, int c3) {
            int tar[] = {c1, c2, c3};
            return matchesAny(tar, arrayLength(tar));
        }
        
        bool matchesAny(int c1, int c2, int c3, int c4) {
            int tar[] = {c1, c2, c3, c4};
            return matchesAny(tar, arrayLength(tar));
        }
        
        bool matchesAny(int c1, int c2, int c3, int c4, int c5) {
            int tar[] = {c1, c2, c3, c4, c5};
            return matchesAny(tar, arrayLength(tar));
        }
        
        bool matchesAny(int c1, int c2, int c3, int c4, int c5, int c6) {
            int tar[] = {c1, c2, c3, c4, c5, c6};
            return matchesAny(tar, arrayLength(tar));
        }
        
        bool matchesAny(const int* arr, const size_t len) {
            if (empty()) return false;
            
            for (size_t i = 0; i < len; ++ i) {
                if (peek() == arr[i]) {
                    return true;
                }
            }
            
            return false;
        }
    
        // the index is start_ based
        size_t nextIndexOf(int c);
        size_t nextIndexOf(const StringRef& seq);
        
        void consumeToEnd(StringBuffer* output);
  
        //static const int EOF = -1;
        static const int eof_ = -1;
    private:
        void readChar();
        
        const CharType* start_;
        const CharType* cur_;
        const CharType* mark_;
        const CharType* end_;
        
        int current_;
        size_t width_;
    };
}

#endif // CSOUP_CHARACTER_READER_H_
