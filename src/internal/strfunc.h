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

namespace csoup {
namespace internal {

//! Custom strlen() which works on different character types.
/*! \tparam Ch Character type (e.g. char, wchar_t, short)
    \param s Null-terminated input string.
    \return Number of characters in the string. 
    \note This has the same semantics as strlen(), the return value is not number of Unicode codepoints.
*/
template <typename Ch>
inline SizeType strLen(const Ch* s) {
    const Ch* p = s;
    while (*p) ++p;
    return SizeType(p - s);
}
    
template <typename Ch>
inline int strCmp(const Ch* sa, const Ch* sb, const SizeType len) {
    return std::memcmp(sa, sb, sizeof(Ch) * len);
}
    
template <typename Ch>
inline int strCmpIgnoreCase(const Ch* sa, const Ch* sb, const SizeType len) {
    int i = 0;
    while (i < len && std::tolower(sa[i]) == std::tolower(sb[i]))
        i ++;
    
    return i == len ? 0 : std::tolower(sa[i]) - std::tolower(sb[i]);
}
    
} // namespace internal
} // namespace csoup

#endif // CSOUP_INTERNAL_STRFUNC_H_
