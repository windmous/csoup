//
//  stringstream.h
//  CSoup
//
//  Created by mac on 11/28/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_STRINGSTREAM_H
#define CSOUP_STRINGSTREAM_H

///////////////////////////////////////////////////////////////////////////////
// Allocators and Encodings

#include "common.h"
#include "allocators.h"
#include "encodings.h"

//! main RapidJSON namespace
namespace csoup {
    
    ///////////////////////////////////////////////////////////////////////////////
    //  Stream
    
    /*! \class rapidjson::Stream
     \brief Concept for reading and writing characters.
     
     For read-only stream, no need to implement putBegin(), put(), flush() and putEnd().
     
     For write-only stream, only need to implement put() and flush().
     
     \code
     concept Stream {
     typename Ch;    //!< Character type of the stream.
     
     //! Read the current character from stream without moving the read cursor.
     Ch peek() const;
     
     //! Read the current character from stream and moving the read cursor to next character.
     Ch take();
     
     //! Get the current read cursor.
     //! \return Number of characters read from start.
     size_t tell();
     
     //! Begin writing operation at the current read pointer.
     //! \return The begin writer pointer.
     Ch* putBegin();
     
     //! Write a character.
     void put(Ch c);
     
     //! flush the buffer.
     void flush();
     
     //! End the writing operation.
     //! \param begin The begin write pointer returned by putBegin().
     //! \return Number of characters written.
     size_t putEnd(Ch* begin);
     }
     \endcode
     */
    
    //! Provides additional information for stream.
    /*!
     By using traits pattern, this type provides a default configuration for stream.
     For custom stream, this type can be specialized for other configuration.
     See TEST(Reader, CustomStringStream) in readertest.cpp for example.
     */
    template<typename Stream>
    struct StreamTraits {
        //! Whether to make local copy of stream for optimization during parsing.
        /*!
         By default, for safety, streams do not use local copy optimization.
         Stream that can be copied fast should specialize this, like StreamTraits<StringStream>.
         */
        enum { copyOptimization = 0 };
    };
    
    //! put N copies of a character to a stream.
    template<typename Stream, typename Ch>
    inline void putN(Stream& stream, Ch c, size_t n) {
        for (size_t i = 0; i < n; i++)
            stream.put(c);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // StringStream
    
    //! Read-only string stream.
    /*! \note implements Stream concept
     */
    template <typename Encoding>
    struct GenericStringStream {
        typedef typename Encoding::Ch Ch;
        
        GenericStringStream(const Ch *src) : src_(src), head_(src) {}
        
        Ch peek() const { return *src_; }
        Ch take() { return *src_++; }
        size_t tell() const { return static_cast<size_t>(src_ - head_); }
        
        Ch* putBegin() { CSOUP_ASSERT(false); return 0; }
        void put(Ch) { CSOUP_ASSERT(false); }
        void flush() { CSOUP_ASSERT(false); }
        size_t putEnd(Ch*) { CSOUP_ASSERT(false); return 0; }
        
        const Ch* src_;     //!< Current read position.
        const Ch* head_;    //!< Original head of the string.
    };
    
    template <typename Encoding>
    struct StreamTraits<GenericStringStream<Encoding> > {
        enum { copyOptimization = 1 };
    };
    
    //! String stream with UTF8 encoding.
    typedef GenericStringStream<UTF8<> > StringStream;
    
    ///////////////////////////////////////////////////////////////////////////////
    // InsituStringStream
    
    //! A read-write string stream.
    /*! This string stream is particularly designed for in-situ parsing.
     \note implements Stream concept
     */
    template <typename Encoding>
    struct GenericInsituStringStream {
        typedef typename Encoding::Ch Ch;
        
        GenericInsituStringStream(Ch *src) : src_(src), dst_(0), head_(src) {}
        
        // Read
        Ch peek() { return *src_; }
        Ch take() { return *src_++; }
        size_t tell() { return static_cast<size_t>(src_ - head_); }
        
        // Write
        void put(Ch c) { CSOUP_ASSERT(dst_ != 0); *dst_++ = c; }
        
        Ch* putBegin() { return dst_ = src_; }
        size_t putEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
        void flush() {}
        
        Ch* push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
        void pop(size_t count) { dst_ -= count; }
        
        Ch* src_;
        Ch* dst_;
        Ch* head_;
    };
    
    template <typename Encoding>
    struct StreamTraits<GenericInsituStringStream<Encoding> > {
        enum { copyOptimization = 1 };
    };
    
    //! Insitu string stream with UTF8 encoding.
    typedef GenericInsituStringStream<UTF8<> > InsituStringStream;
    
    ///////////////////////////////////////////////////////////////////////////////
    // Type
    
    //! Type of JSON value
    enum Type {
        kNullType = 0,      //!< null
        kFalseType = 1,     //!< false
        kTrueType = 2,      //!< true
        kObjectType = 3,    //!< object
        kArrayType = 4,     //!< array 
        kStringType = 5,    //!< string
        kNumberType = 6     //!< number
    };
    
} // namespace rapidjson


#endif
