#ifndef CSOUP_CSOUP_STRING_H_
#define CSOUP_CSOUP_STRING_H_

#include <cstring>
#include "common.h"
#include "allocators.h"
#include "stringref.h"
#include "../internal/strfunc.h"

// NO_EXCEPTION should be reconsidered carefully

namespace csoup {
    
    class String;
    namespace internal {
        extern void destroy(String* obj, Allocator* allocator);
    }

class String {
public:
    static const int CSOUP_STRING_COMPARE_SUPPORTED = 1;
    
    template<size_t N>
    String(const CharType (&str)[N], Allocator* allocator) CSOUP_NOEXCEPT
        : data_(NULL), length_(N-1), allocator_(allocator) {
        //CSOUP_ASSERT(length_ <= MaxStringLength);
        CSOUP_ASSERT(allocator != NULL);
    
        copyString(str, allocator);
    }
    
    String(const StringRef& str, Allocator* allocator)
    : data_(NULL), length_(str.size()), allocator_(allocator) {
        CSOUP_ASSERT(allocator != NULL);
        //CSOUP_ASSERT(str.data() != NULL);
        copyString(str.data(), allocator);
    }

    explicit String(const CharType* str, Allocator* allocator)
        : data_(NULL), length_(internal::strLen(str)), allocator_(allocator){
            CSOUP_ASSERT(str != NULL);
            //CSOUP_ASSERT(length_ <= MaxStringLength);
            CSOUP_ASSERT(allocator != NULL);
            
            copyString(str, allocator);
    }

    String(const CharType* str, const size_t len, Allocator* allocator)
        : data_(str), length_(len), allocator_(allocator) {
        CSOUP_ASSERT(str != NULL);
        //CSOUP_ASSERT(len <= MaxStringLength);
        CSOUP_ASSERT(allocator != NULL);
            
        copyString(str, allocator);
    }
    
    String(const String& str, Allocator* allocator) :
        data_(NULL), length_(str.size()), allocator_(allocator) {
        // prevent str from a destroied string object
        CSOUP_ASSERT(str.data() != NULL);
        CSOUP_ASSERT(allocator != NULL);
        
        copyString(str.data(), allocator);
    }
    
    ~String() {
        if (!allocator_) return ;
        allocator_->free(data_);
        allocator_ = NULL;
    }
    
    StringRef ref() const {
        return StringRef(data(), size());
    }
    

    //! implicit conversion to plain CharType pointer
    operator const CharType *() const { return data_; }
    
    const CharType* data() const {return data_; }
    
    const size_t size() const {return length_;}
    
    operator StringRef () const {
        return StringRef(data(), size());
    }
    
    //friend String deepcopy(const String& obj, Allocator* allocator);
    friend void internal::destroy(String* obj, Allocator* allocator);
private:
//    static size_t CopyBitMask;
//    static size_t InvertedCopyBitMask;
//    static size_t MaxStringLength;
    
//    String(const CharType* str, const size_t len) : data_(str), length_(len) {
//        CSOUP_ASSERT(str != NULL);
//        //CSOUP_ASSERT(len <= MaxStringLength);
//    }
    
    void copyString(const CharType* str, Allocator* allocator) {
        size_t buffSize = sizeof(CharType) * length_;
        if (buffSize == 0) {
            data_ = "";
        } else {
            CharType* buffer = static_cast<CharType*>(allocator->malloc(buffSize));
            std::memcpy(buffer, str, buffSize);
            data_ = buffer;
        }
    }
    
    const CharType* data_; //!< plain CharType pointer
    size_t length_; //!< length of the string (excluding the trailing NULL terminator)
    Allocator* allocator_;
    
    bool operator == (const String&);
    //! Disallow copy-assignment
    String operator=(const String&);
    //! Disallow construction from non-const array
    template<size_t N>
    String(CharType (&str)[N]) /* = delete */;
    
    String(const String& obj);
};
    
    namespace internal {
//        inline String deepcopy(const String& obj, Allocator* allocator) {
//            const size_t data_size = sizeof(CharType) * obj.size();
//            CharType* s = static_cast<CharType*>(allocator->malloc(data_size));
//            std::memcpy(s, obj.data(), obj.size());
//            
//            String ret(s, obj.size());
//            ret.length_ |= String::CopyBitMask;
//            return ret;
//        }
        
        // this method only  guarantees that all memory resources would
        // be freed. Don't think that
        inline void destroy(String* obj, Allocator* allocator) {
//            if (obj->data_ && obj->data_[0] != '\0') {
//                allocator->free(static_cast<const void*>(obj->data()));
//            }
//            
//            obj->length_ = 0;
//            obj->data_ = NULL;
        }
    }

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.
    \tparam CharType Character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericString string reference object
    \relatesalso GenericString

    \see GenericValue::GenericValue(StringRefType), GenericValue::operator=(StringRefType), GenericValue::SetString(StringRefType), GenericValue::PushBack(StringRefType, Allocator&), GenericValue::AddMember
*/
//template<typename CharType>
//inlineString String(const CharType* str) {
//    returnString(str, internal::strLen(str));
//}

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
    \return GenericString string reference object
    \relatesalso GenericString
*/
//template<typename CharType>
//inlineString String(const CharType* str, size_t length) {
//    returnString(str, size_t(length));
//}

#if CSOUP_HAS_STDSTRING
//! Mark a string object as constant string
/*! Mark a string object (e.g. \c std::string) as a "string literal".
    This function can be used to avoid copying a string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericString string reference object
    \relatesalso GenericString
    \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
*/
//template<typename CharType>
//inlineString String(const std::basic_string<CharType>& str) {
//    returnString(str.data(), size_t(str.size()));
//}
#endif
}

#endif // CSOUP_CSOUP_STRING_H_