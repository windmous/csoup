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

#ifndef CSOUP_STRINGREF_H_
#define CSOUP_STRINGREF_H_

#include "common.h"
#include "../internal/strfunc.h"

namespace csoup {

///////////////////////////////////////////////////////////////////////////////
// GenericStringRef

//! Reference to a constant string (not taking a copy)
/*!
    \tparam CharType character type of the string

    This helper class is used to automatically infer constant string
    references for string literals, especially from \c const \b (!)
    character arrays.

    The main use is for creating JSON string values without copying the
    source string via an \ref Allocator.  This requires that the referenced
    string pointers have a sufficient lifetime, which exceeds the lifetime
    of the associated GenericValue.

    \b Example
    \code
    Value v("foo");   // ok, no need to copy & calculate length
    const char foo[] = "foo";
    v.SetString(foo); // ok

    const char* bar = foo;
    // Value x(bar); // not ok, can't rely on bar's lifetime
    Value x(StringRef(bar)); // lifetime explicitly guaranteed by user
    Value y(StringRef(bar, 3));  // ok, explicitly pass length
    \endcode

    \see StringRef, GenericValue::SetString
*/
template<typename CharType>
struct GenericStringRef {
    typedef CharType Ch; //!< character type of the string

    //! Create string reference from \c const character array
    /*!
        This constructor implicitly creates a constant string reference from
        a \c const character array.  It has better performance than
        \ref StringRef(const CharType*) by inferring the string \ref length
        from the array length, and also supports strings containing null
        characters.

        \tparam N length of the string, automatically inferred

        \param str Constant character array, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note Constant complexity.
        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
    template<SizeType N>
    GenericStringRef(const CharType (&str)[N]) CSOUP_NOEXCEPT
        : s(str), length(N-1) {}

    //! Explicitly create string reference from \c const character pointer
    /*!
        This constructor can be used to \b explicitly  create a reference to
        a constant string pointer.

        \see StringRef(const CharType*)

        \param str Constant character pointer, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
    explicit GenericStringRef(const CharType* str)
        : s(str), length(internal::strLen(str)){ CSOUP_ASSERT(s != NULL); }

    //! Create constant string reference from pointer and length
    /*! \param str constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
        \param len length of the string, excluding the trailing NULL terminator

        \post \ref s == str && \ref length == len
        \note Constant complexity.
     */
    GenericStringRef(const CharType* str, SizeType len)
        : s(str), length(len) { CSOUP_ASSERT(s != NULL); }
    
    bool operator == (const GenericStringRef<Ch>& obj) const {
        return this->length == obj.length && strCmp(this->s, obj.s) == 0;
    }
    
    bool equalsIgnoreCase(const GenericStringRef<Ch>& obj) const {
        return this->length == obj.length && strCmpIgnoreCase(this->s, obj.s) == 0;
    }

    //! implicit conversion to plain CharType pointer
    operator const Ch *() const { return s; }

    const Ch* const s; //!< plain CharType pointer
    const SizeType length; //!< length of the string (excluding the trailing NULL terminator)

    template <typename Allocator>
    GenericStringRef<Ch>* clone(Allocator* allocator) const {
        
    }
private:
    //! Disallow copy-assignment
    GenericStringRef operator=(const StringRef&);
    //! Disallow construction from non-const array
    template<SizeType N>
    GenericStringRef(CharType (&str)[N]) /* = delete */;
};

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.
    \tparam CharType Character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef

    \see GenericValue::GenericValue(StringRefType), GenericValue::operator=(StringRefType), GenericValue::SetString(StringRefType), GenericValue::PushBack(StringRefType, Allocator&), GenericValue::AddMember
*/
template<typename CharType>
inline StringRef<CharType> makeStringRef(const CharType* str) {
    return StringRef<CharType>(str, internal::strLen(str));
}

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    This version has better performance with supplied length, and also
    supports string containing null characters.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \param length The length of source string.
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
*/
template<typename CharType>
inline StringRef<CharType> makeStringRef(const CharType* str, size_t length) {
    return StringRef<CharType>(str, SizeType(length));
}

#if CSOUP_HAS_STDSTRING
//! Mark a string object as constant string
/*! Mark a string object (e.g. \c std::string) as a "string literal".
    This function can be used to avoid copying a string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
    \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
*/
template<typename CharType>
inline StringRef<CharType> makeStringRef(const std::basic_string<CharType>& str) {
    return StringRef<CharType>(str.data(), SizeType(str.size()));
}
#endif
}

#endif // CSOUP_STRINGREF_H_