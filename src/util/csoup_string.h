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
    typedef Allocator AllocatorType;
    static const int CSOUP_STRING_COMPARE_SUPPORTED = 1;
    
    template<size_t N>
    String(const CharType (&str)[N], Allocator* allocator) CSOUP_NOEXCEPT
    : type_(CSOUP_UNDEFINED_STRING) {
        
        copyString(str, N - 1, allocator);
    }
    
    String(const StringRef& str, Allocator* allocator)
    : type_(CSOUP_UNDEFINED_STRING) {
        copyString(str, str.size(), allocator);
    }

    explicit String(const CharType* str, Allocator* allocator)
    : type_(CSOUP_UNDEFINED_STRING) {
        copyString(str, internal::strLen(str), allocator);
    }

    String(const CharType* str, const size_t len, Allocator* allocator)
    : type_(CSOUP_UNDEFINED_STRING) {
        copyString(str, len, allocator);
    }
    
    String(const String& str, Allocator* allocator) :
        type_(CSOUP_UNDEFINED_STRING) {
        copyString(str.data(), str.size(), allocator);
    }
    
    ~String() {
        if (type_ == CSOUP_LONG_STRING) {
            data_.ls_.allocator_->free(data_.ls_.str_);
            type_ = CSOUP_UNDEFINED_STRING;
        }
    }
    
    StringRef ref() const {
        return StringRef(data(), size());
    }
    

    //! implicit conversion to plain CharType pointer
    operator const CharType *() const {
        return data();
    }
    
    const CharType* data() const {
        CSOUP_ASSERT(type_ != CSOUP_UNDEFINED_STRING);
        return type_ == CSOUP_SHORT_STRING ? data_.ss_.str_ : data_.ls_.str_;
    }
    
    const size_t size() const {
        CSOUP_ASSERT(type_ != CSOUP_UNDEFINED_STRING);
        return type_ == CSOUP_SHORT_STRING ? data_.ss_.length() : data_.ls_.length_;
    }
    
    operator StringRef () const {
        return StringRef(data(), size());
    }

    Allocator* allocator() {
        CSOUP_ASSERT(type_ != CSOUP_UNDEFINED_STRING);
        if (type_ == CSOUP_SHORT_STRING)    return globalDumbAllocator();
        else                                return data_.ls_.allocator_;
    }
    
    // friend String deepcopy(const String& obj, Allocator* allocator);
    // friend void internal::destroy(String* obj, Allocator* allocator);
private:
//    static size_t CopyBitMask;
//    static size_t InvertedCopyBitMask;
//    static size_t MaxStringLength;
    
    void copyString(const CharType* str, size_t len, Allocator* allocator) {
        CSOUP_ASSERT(str        != NULL);
        CSOUP_ASSERT(allocator  != NULL);
        
        if (ShortString::usable(len))   copyShortString(str, len);
        else                            copyLongString(str, len, allocator);
    }
    
    void copyShortString(const CharType* str, size_t len) {
        type_                   = CSOUP_SHORT_STRING;
        size_t buffSize         = sizeof(CharType) * len;
        std::memcpy(data_.ss_.str_, str, buffSize);
        data_.ss_.setLength(len);
    }
    
    void copyLongString(const CharType* str, size_t len, Allocator* allocator) {
        type_                   = CSOUP_LONG_STRING;
        data_.ls_.length_       = len;
        data_.ls_.allocator_    = allocator;
        size_t buffSize         = sizeof(CharType) * len;
        
        if (buffSize == 0) {
            data_.ls_.str_ = "";
        } else {
            CharType* buffer = static_cast<CharType*>(allocator->malloc(buffSize));
            std::memcpy(buffer, str, buffSize);
            data_.ls_.str_ = buffer;
        }
    }
    
    enum StringTypeEnum { CSOUP_SHORT_STRING, CSOUP_LONG_STRING, CSOUP_UNDEFINED_STRING};
    
    struct LongString {
        const CharType* str_; //!< plain CharType pointer
        size_t length_; //!< length of the string (excluding the trailing NULL terminator)
        Allocator* allocator_;
    };
    
    struct ShortString {
        enum { MaxChars = sizeof(LongString) / sizeof(CharType), MaxSize = MaxChars - 1, LenPos = MaxSize };
        CharType str_[MaxChars];
        
        inline static bool usable(size_t len) { return (MaxSize >= len); }
        inline void     setLength(size_t len) { str_[LenPos] = (CharType)(MaxSize - len); }
        inline size_t length() const          { return (CharType)(MaxSize - str_[LenPos]); }
    };
    
    union {
        struct LongString  ls_;
        struct ShortString ss_;
    } data_;
    
    StringTypeEnum type_;
    
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