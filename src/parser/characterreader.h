//
//  characterreader.h
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_CHARACTER_READER_H_
#define CSOUP_CHARACTER_READER_H_

#include "../util/common.h"
#include "../util/stringref.h"
namespace csoup {
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
            return start_ == end_;
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
        
        
        
        static const int EOF = -1;
    private:
        void readChar();
        
        const CharType* start_;
        const CharType* cur_;
        const CharType* mark_;
        const CharType* end_;
        
        int current_;
        int width_;
    };
}

#endif // CSOUP_CHARACTER_READER_H_
