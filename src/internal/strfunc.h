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

#ifndef CSOUP_INTERNAL_STRFUNC_H_
#define CSOUP_INTERNAL_STRFUNC_H_

#include <cstring>
#include <cctype>
#include "../util/common.h"

namespace csoup {
    namespace internal {

        //! Custom strlen() which works on different character types.
        /*! \tparam Ch Character type (e.g. char, wchar_t, short)
            \param s Null-terminated input string.
            \return Number of characters in the string. 
            \note This has the same semantics as strlen(), the return value is not number of Unicode codepoints.
        */
        template <typename Ch>
        inline size_t strLen(const Ch* s) {
            const Ch* p = s;
            while (*p) ++p;
            return size_t(p - s);
        }

        template <typename Ch>
        inline int strCmp(const Ch* sa, const Ch* sb, const size_t len) {
            return std::memcmp(sa, sb, sizeof(Ch) * len);
        }
            
        // Be careful!! This is not an bad implementation!
        template <typename Ch>
        inline int strCmpIgnoreCase(const Ch* sa, const Ch* sb, const size_t len) {
            size_t i = 0;
            while (i < len && std::tolower(sa[i]) == std::tolower(sb[i]))
                i ++;
            
            return i == len ? 0 : std::tolower(sa[i]) - std::tolower(sb[i]);
        }
            
        template <typename C1, typename C2>
        inline bool strEquals(const C1& s1, const C2& s2) {
            // A stupid static_assert! Must change this with an meta-programming library.
            CSOUP_STATIC_ASSERT(C1::CSOUP_STRING_COMPARE_SUPPORTED == 1);
            CSOUP_STATIC_ASSERT(C2::CSOUP_STRING_COMPARE_SUPPORTED == 1);
            
            return (reinterpret_cast<const char*>(&s1) == reinterpret_cast<const char*>(&s2)) ||
                    ((s1.size() == s2.size() && 0 == strCmp(s1.data(), s2.data(), s1.size())));
        }

        template <typename C1, typename C2>
        inline bool strEqualsIgnoreCase(const C1& s1, const C2& s2) {
            // A stupid static_assert! Must change this with an meta-programming library.
            CSOUP_STATIC_ASSERT(C1::CSOUP_STRING_COMPARE_SUPPORTED == 1);
            CSOUP_STATIC_ASSERT(C2::CSOUP_STRING_COMPARE_SUPPORTED == 1);
            
            return (reinterpret_cast<const char*>(&s1) == reinterpret_cast<const char*>(&s2)) ||
            ((s1.size() == s2.size() && 0 == strCmpIgnoreCase(s1.data(), s2.data(), s1.size())));
        }
    
    } // namespace internal
} // namespace csoup

#endif // CSOUP_INTERNAL_STRFUNC_H_
