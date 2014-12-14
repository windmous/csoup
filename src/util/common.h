// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef CSOUP_CSOUP_H_
#define CSOUP_CSOUP_H_

// Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
// Version 0.1

/*!\file rapidjson.h
    \brief common definitions and configuration

    \see CSOUP_CONFIG
 */

/*! \defgroup CSOUP_CONFIG RapidJSON configuration
    \brief Configuration macros for library features

    Some RapidJSON features are configurable to adapt the library to a wide
    variety of platforms, environments and usage scenarios.  Most of the
    features can be configured in terms of overriden or predefined
    preprocessor macros at compile-time.

    Some additional customization is available in the \ref CSOUP_ERRORS APIs.

    \note These macros should be given on the compiler command-line
          (where applicable)  to avoid inconsistent values when compiling
          different translation units of a single application.
 */

#include <cstdlib>  // malloc(), realloc(), free(), size_t
#include <cstring>  // memset(), memcpy(), memmove(), memcmp()
#include <new>

///////////////////////////////////////////////////////////////////////////////
// CSOUP_NO_INT64DEFINE

/*! \def CSOUP_NO_INT64DEFINE
    \ingroup CSOUP_CONFIG
    \brief Use external 64-bit integer types.

    RapidJSON requires the 64-bit integer types \c int64_t and  \c uint64_t types
    to be available at global scope.

    If users have their own definition, define CSOUP_NO_INT64DEFINE to
    prevent RapidJSON from defining its own types.
*/
#ifndef CSOUP_NO_INT64DEFINE
//!@cond CSOUP_HIDDEN_FROM_DOXYGEN
#ifdef _MSC_VER
#include "msinttypes/stdint.h"
#include "msinttypes/inttypes.h"
#else
// Other compilers should have this.
#include <stdint.h>
#include <inttypes.h>
#endif
//!@endcond
#ifdef CSOUP_DOXYGEN_RUNNING
#define CSOUP_NO_INT64DEFINE
#endif
#endif // CSOUP_NO_INT64TYPEDEF

///////////////////////////////////////////////////////////////////////////////
// CSOUP_FORCEINLINE

#ifndef CSOUP_FORCEINLINE
//!@cond CSOUP_HIDDEN_FROM_DOXYGEN
#ifdef _MSC_VER
#define CSOUP_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ >= 4
#define CSOUP_FORCEINLINE __attribute__((always_inline))
#else
#define CSOUP_FORCEINLINE
#endif
//!@endcond
#endif // CSOUP_FORCEINLINE

///////////////////////////////////////////////////////////////////////////////
// CSOUP_ENDIAN
#define CSOUP_LITTLEENDIAN  0   //!< Little endian machine
#define CSOUP_BIGENDIAN     1   //!< Big endian machine

//! Endianness of the machine.
/*!
    \def CSOUP_ENDIAN
    \ingroup CSOUP_CONFIG

    GCC 4.6 provided macro for detecting endianness of the target machine. But other
    compilers may not have this. User can define CSOUP_ENDIAN to either
    \ref CSOUP_LITTLEENDIAN or \ref CSOUP_BIGENDIAN.

    Default detection implemented with reference to
    \li https://gcc.gnu.org/onlinedocs/gcc-4.6.0/cpp/Common-Predefined-Macros.html
    \li http://www.boost.org/doc/libs/1_42_0/boost/detail/endian.hpp
*/
#ifndef CSOUP_ENDIAN
// Detect with GCC 4.6's macro
#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define CSOUP_ENDIAN CSOUP_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define CSOUP_ENDIAN CSOUP_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define CSOUP_ENDIAN.
#    endif // __BYTE_ORDER__
// Detect with GLIBC's endian.h
#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define CSOUP_ENDIAN CSOUP_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define CSOUP_ENDIAN CSOUP_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define CSOUP_ENDIAN.
#   endif // __GLIBC__
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define CSOUP_ENDIAN CSOUP_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define CSOUP_ENDIAN CSOUP_BIGENDIAN
// Detect with architecture macros
#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define CSOUP_ENDIAN CSOUP_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define CSOUP_ENDIAN CSOUP_LITTLEENDIAN
#  elif defined(CSOUP_DOXYGEN_RUNNING)
#    define CSOUP_ENDIAN
#  else
#    error Unknown machine endianess detected. User needs to define CSOUP_ENDIAN.   
#  endif
#endif // CSOUP_ENDIAN

///////////////////////////////////////////////////////////////////////////////
// CSOUP_64BIT

//! Whether using 64-bit architecture
#ifndef CSOUP_64BIT
#if defined(__LP64__) || defined(_WIN64)
#define CSOUP_64BIT 1
#else
#define CSOUP_64BIT 0
#endif
#endif // CSOUP_64BIT

///////////////////////////////////////////////////////////////////////////////
// CSOUP_ALIGN

//! Data alignment of the machine.
/*! \ingroup CSOUP_CONFIG
    \param x pointer to align

    Some machines require strict data alignment. Currently the default uses 4 bytes
    alignment. User can customize by defining the CSOUP_ALIGN function macro.,
*/
#ifndef CSOUP_ALIGN
#define CSOUP_ALIGN(x) ((x + 3u) & ~3u)
#endif

///////////////////////////////////////////////////////////////////////////////
// CSOUP_UINT64_C2

//! Construct a 64-bit literal by a pair of 32-bit integer.
/*!
    64-bit literal with or without ULL suffix is prone to compiler warnings.
    UINT64_C() is C macro which cause compilation problems.
    Use this macro to define 64-bit constants by a pair of 32-bit integer.
*/
#ifndef CSOUP_UINT64_C2
#define CSOUP_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#endif

///////////////////////////////////////////////////////////////////////////////
// CSOUP_SSE2/CSOUP_SSE42/CSOUP_SIMD

/*! \def CSOUP_SIMD
    \ingroup CSOUP_CONFIG
    \brief Enable SSE2/SSE4.2 optimization.

    RapidJSON supports optimized implementations for some parsing operations
    based on the SSE2 or SSE4.2 SIMD extensions on modern Intel-compatible
    processors.

    To enable these optimizations, two different symbols can be defined;
    \code
    // Enable SSE2 optimization.
    #define CSOUP_SSE2

    // Enable SSE4.2 optimization.
    #define CSOUP_SSE42
    \endcode

    \c CSOUP_SSE42 takes precedence, if both are defined.

    If any of these symbols is defined, RapidJSON defines the macro
    \c CSOUP_SIMD to indicate the availability of the optimized code.
*/
#if defined(CSOUP_SSE2) || defined(CSOUP_SSE42) \
    || defined(CSOUP_DOXYGEN_RUNNING)
#define CSOUP_SIMD
#endif

///////////////////////////////////////////////////////////////////////////////
// CSOUP_NO_SIZETYPEDEFINE

#ifndef CSOUP_NO_SIZETYPEDEFINE
/*! \def CSOUP_NO_SIZETYPEDEFINE
    \ingroup CSOUP_CONFIG
    \brief User-provided \c SizeType definition.

    In order to avoid using 32-bit size types for indexing strings and arrays,
    define this preprocessor symbol and provide the type rapidjson::SizeType
    before including RapidJSON:
    \code
    #define CSOUP_NO_SIZETYPEDEFINE
    namespace rapidjson { typedef ::std::size_t SizeType; }
    #include "rapidjson/..."
    \endcode

    \see rapidjson::SizeType
*/
#ifdef CSOUP_DOXYGEN_RUNNING
#define CSOUP_NO_SIZETYPEDEFINE
#endif
namespace csoup {
//! Size type (for string lengths, array sizes, etc.)
/*! RapidJSON uses 32-bit array/string indices even on 64-bit platforms,
    instead of using \c size_t. Users may override the SizeType by defining
    \ref CSOUP_NO_SIZETYPEDEFINE.
*/
//typedef unsigned SizeType;
} // namespace csoup
#endif

// always import std::size_t to csoup namespace
namespace csoup {
using std::size_t;
} // namespace csoup

namespace csoup {
    typedef char CharType;
}

///////////////////////////////////////////////////////////////////////////////
// CSOUP_ASSERT

//! Assertion.
/*! \ingroup CSOUP_CONFIG
    By default, csoup uses C \c assert() for internal assertions.
    User can override it by defining CSOUP_ASSERT(x) macro.

    \note Parsing errors are handled and can be customized by the
          \ref CSOUP_ERRORS APIs.
*/
#ifndef CSOUP_ASSERT
#include <cassert>
#define CSOUP_ASSERT(x) assert(x)
#endif // CSOUP_ASSERT

///////////////////////////////////////////////////////////////////////////////
// CSOUP_STATIC_ASSERT

// Adopt from boost
#ifndef CSOUP_STATIC_ASSERT
//!@cond CSOUP_HIDDEN_FROM_DOXYGEN
namespace csoup {

template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct StaticAssertTest {};
} // namespace csoup

#define CSOUP_JOIN(X, Y) CSOUP_DO_JOIN(X, Y)
#define CSOUP_DO_JOIN(X, Y) CSOUP_DO_JOIN2(X, Y)
#define CSOUP_DO_JOIN2(X, Y) X##Y

#if defined(__GNUC__)
#define CSOUP_STATIC_ASSERT_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define CSOUP_STATIC_ASSERT_UNUSED_ATTRIBUTE 
#endif
//!@endcond

/*! \def CSOUP_STATIC_ASSERT
    \brief (Internal) macro to check for conditions at compile-time
    \param x compile-time condition
    \hideinitializer
 */
#define CSOUP_STATIC_ASSERT(x) typedef ::csoup::StaticAssertTest<\
    sizeof(::csoup::STATIC_ASSERTION_FAILURE<bool(x) >)>\
    CSOUP_JOIN(StaticAssertTypedef, __LINE__) CSOUP_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers

//!@cond CSOUP_HIDDEN_FROM_DOXYGEN

#define CSOUP_MULTILINEMACRO_BEGIN do {  
#define CSOUP_MULTILINEMACRO_END \
} while((void)0, 0)

// adopted from Boost
#define CSOUP_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))

// token stringification
#define CSOUP_STRINGIFY(x) CSOUP_DO_STRINGIFY(x)
#define CSOUP_DO_STRINGIFY(x) #x

///////////////////////////////////////////////////////////////////////////////
// CSOUP_DIAG_PUSH/POP, CSOUP_DIAG_OFF

#if defined(__GNUC__)
#define CSOUP_GNUC \
    CSOUP_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#endif

#if defined(__clang__) || (defined(CSOUP_GNUC) && CSOUP_GNUC >= CSOUP_VERSION_CODE(4,2,0))

#define CSOUP_PRAGMA(x) _Pragma(CSOUP_STRINGIFY(x))
#define CSOUP_DIAG_PRAGMA(x) CSOUP_PRAGMA(GCC diagnostic x)
#define CSOUP_DIAG_OFF(x) \
    CSOUP_DIAG_PRAGMA(ignored CSOUP_STRINGIFY(CSOUP_JOIN(-W,x)))

// push/pop support in Clang and GCC>=4.6
#if defined(__clang__) || (defined(CSOUP_GNUC) && CSOUP_GNUC >= CSOUP_VERSION_CODE(4,6,0))
#define CSOUP_DIAG_PUSH CSOUP_DIAG_PRAGMA(push)
#define CSOUP_DIAG_POP  CSOUP_DIAG_PRAGMA(pop)
#else // GCC >= 4.2, < 4.6
#define CSOUP_DIAG_PUSH /* ignored */
#define CSOUP_DIAG_POP /* ignored */
#endif

#elif defined(_MSC_VER)

// pragma (MSVC specific)
#define CSOUP_PRAGMA(x) __pragma(x)
#define CSOUP_DIAG_PRAGMA(x) CSOUP_PRAGMA(warning(x))

#define CSOUP_DIAG_OFF(x) CSOUP_DIAG_PRAGMA(disable: x)
#define CSOUP_DIAG_PUSH CSOUP_DIAG_PRAGMA(push)
#define CSOUP_DIAG_POP  CSOUP_DIAG_PRAGMA(pop)

#else

#define CSOUP_DIAG_OFF(x) /* ignored */
#define CSOUP_DIAG_PUSH   /* ignored */
#define CSOUP_DIAG_POP    /* ignored */

#endif // CSOUP_DIAG_*

///////////////////////////////////////////////////////////////////////////////
// C++11 features

#ifndef CSOUP_HAS_CXX11_RVALUE_REFS
#if defined(__clang__)
#define CSOUP_HAS_CXX11_RVALUE_REFS __has_feature(cxx_rvalue_references)
#elif (defined(CSOUP_GNUC) && (CSOUP_GNUC >= CSOUP_VERSION_CODE(4,3,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1600)

#define CSOUP_HAS_CXX11_RVALUE_REFS 1
#else
#define CSOUP_HAS_CXX11_RVALUE_REFS 0
#endif
#endif // CSOUP_HAS_CXX11_RVALUE_REFS

#ifndef CSOUP_HAS_CXX11_NOEXCEPT
#if defined(__clang__)
#define CSOUP_HAS_CXX11_NOEXCEPT __has_feature(cxx_noexcept)
#elif (defined(CSOUP_GNUC) && (CSOUP_GNUC >= CSOUP_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__))
//    (defined(_MSC_VER) && _MSC_VER >= ????) // not yet supported
#define CSOUP_HAS_CXX11_NOEXCEPT 1
#else
#define CSOUP_HAS_CXX11_NOEXCEPT 0
#endif
#endif
#if CSOUP_HAS_CXX11_NOEXCEPT
#define CSOUP_NOEXCEPT noexcept
#else
#define CSOUP_NOEXCEPT /* noexcept */
#endif // CSOUP_HAS_CXX11_NOEXCEPT

// no automatic detection, yet
#ifndef CSOUP_HAS_CXX11_TYPETRAITS
#define CSOUP_HAS_CXX11_TYPETRAITS 0
#endif

//#define CSOUP_ARRAY_LENGTH(ArrayName) (sizeof(ArrayName) / sizeof(*ArrayName))

//!@endcond
namespace csoup {
    ///////////////////////////////////////////////////////////////////////////////
    // Utility functions
    template <typename Allocator, typename T>
    void destroy(T** ptr, Allocator* allocator) {
        if (*ptr == NULL) return;
        (*ptr)->~T();
        allocator->free(*ptr);
        *ptr = NULL;
    }
    
    template <typename T>
    void destroy(T** ptr) {
        if (*ptr == NULL) return;
        
        typename T::AllocatorType* allocator = (*ptr)->allocator();
        (*ptr)->~T();
        allocator->free(*ptr);
        *ptr = NULL;
    }

    template <typename T, size_t N>
    size_t arrayLength(T (&arr)[N]) {
        return N;
    }
}



#endif // CSOUP_CSOUP_H_
