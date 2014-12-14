#ifndef CSOUP_CSOUP_STRINGREF_H_
#define CSOUP_CSOUP_STRINGREF_H_

#include <cstring>
#include "common.h"
#include "allocators.h"
#include "../internal/strfunc.h"

// NO_EXCEPTION should be reconsidered carefully

namespace csoup {
    
    class StringRef {
    public:
        // this is a trait attribute;
        static const int CSOUP_STRING_COMPARE_SUPPORTED = 1;
        
        template<size_t N>
        StringRef(const CharType (&str)[N]) CSOUP_NOEXCEPT
        : data_(NULL), length_(N-1) {
        }
        
        explicit StringRef(const CharType* str)
        : data_(str), length_(internal::strLen(str)){
            CSOUP_ASSERT(str != NULL);
        }
        
        StringRef(const CharType* str, const size_t len)
        : data_(str), length_(len) {
            CSOUP_ASSERT(str != NULL);
        }
        
        StringRef(const StringRef& str) : data_(str.data()), length_(str.size()){
            CSOUP_ASSERT(str.data() != NULL);
        }
        
        CharType at(size_t index) const {
            CSOUP_ASSERT(index < size());
            return data_[index];
        }
        
        //! implicit conversion to plain CharType pointer
        operator const CharType *() const { return data_; }
        
        const CharType* data() const {return data_; }
        
        const size_t size() const {return length_;}
    private:
        const CharType* const data_; //!< plain CharType pointer
        const size_t length_; //!< length of the string (excluding the trailing NULL terminator)
        
        //! Disallow copy-assignment
        StringRef operator=(const StringRef&);
        
        bool operator == (const StringRef&);
        
        //! Disallow construction from non-const array
        template<size_t N>
        StringRef(CharType (&str)[N]) /* = delete */;
    };
}

#endif // CSOUP_CSOUP_STRING_H_