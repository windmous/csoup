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

#ifndef CSOUP_INTERNAL_STACK_H_
#define CSOUP_INTERNAL_STACK_H_
#include "bytearray.h"

namespace csoup {
    namespace internal {
        
        ///////////////////////////////////////////////////////////////////////////////
        // Stack
        
        //! A type-unsafe stack for storing different types of data.
        /*! \tparam Allocator Allocator for allocating stack memory.
         */
        template <typename Allocator>
        class Stack {
        public:
            // Optimization note: Do not allocate memory for stack_ in constructor.
            // Do it lazily when first Push() -> Expand() -> Resize().
            Stack(Allocator* allocator, size_t stackCapacity) : data_(allocator, stackCapacity) {
                CSOUP_ASSERT(stackCapacity > 0);

            }
            
            ~Stack() {
            }
            
            void clear() { data_.clear(); }
            
            void shrinkToFit() {
                data_.shrinkToFit();
            }
            
            // Optimization note: try to minimize the size of this function for force inline.
            // Expansion is run very infrequently, so it is moved to another (probably non-inline) function.
            template<typename T>
            CSOUP_FORCEINLINE T* push() {
                return data_.template push<T>(1);
            }
            
            template<typename T>
            CSOUP_FORCEINLINE void push(const T& ele) {
                new (data_.template push<T>(1)) T(ele);
            }
            
            template <typename T>
            T* pop() {
                return data_.template pop<T>(1);
            }
            
            template <typename T>
            void popAndDestroy() {
                pop<T>()->~T();
            }
            
            template <typename T>
            T* top() {
                return data_.template top<T>();
            }
            
            template<typename T>
            T* bottom() { return data_.template bottom<T>(); }
            
            Allocator& getAllocator() { return data_.getAllocator(); }
            bool empty() const { return data_.empty(); }
            size_t getSize() const { return data_.getSize(); }
            size_t getCapacity() const { return data_.getCapacity(); }
            
        private:
                        // Prohibit copy constructor & assignment operator.
            Stack(const Stack&);
            Stack& operator=(const Stack&);
            
            ByteArray<Allocator> data_;
        };
        
    } // namespace internal
} // namespace rapidjson

#endif // CSOUP_STACK_H_
