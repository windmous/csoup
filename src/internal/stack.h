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
#include "../util/common.h"

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
            Stack(Allocator* allocator, size_t stackCapacity) : allocator_(allocator), ownAllocator(0), stack_(0), stackTop_(0), stackEnd_(0), initialCapacity_(stackCapacity) {
                CSOUP_ASSERT(stackCapacity > 0);
                if (!allocator_)
                    ownAllocator = allocator_ = new Allocator();
            }
            
            ~Stack() {
                Allocator::free(stack_);
                delete ownAllocator; // Only delete if it is owned by the stack
            }
            
            void clear() { stackTop_ = stack_; }
            
            void shrinkToFit() {
                if (empty()) {
                    // If the stack is empty, completely deallocate the memory.
                    Allocator::free(stack_);
                    stack_ = 0;
                    stackTop_ = 0;
                    stackEnd_ = 0;
                }
                else
                    resize(getSize());
            }
            
            // Optimization note: try to minimize the size of this function for force inline.
            // Expansion is run very infrequently, so it is moved to another (probably non-inline) function.
            template<typename T>
            CSOUP_FORCEINLINE T* push(size_t count = 1) {
                // Expand the stack if needed
                if (stackTop_ + sizeof(T) * count >= stackEnd_)
                    expand<T>(count);
                
                T* ret = reinterpret_cast<T*>(stackTop_);
                stackTop_ += sizeof(T) * count;
                return ret;
            }
            
            template<typename T>
            T* pop(size_t count) {
                CSOUP_ASSERT(getSize() >= count * sizeof(T));
                stackTop_ -= count * sizeof(T);
                return reinterpret_cast<T*>(stackTop_);
            }
            
            template<typename T>
            T* top() {
                CSOUP_ASSERT(getSize() >= sizeof(T));
                return reinterpret_cast<T*>(stackTop_ - sizeof(T));
            }
            
            template<typename T>
            T* bottom() { return (T*)stack_; }
            
            Allocator& getAllocator() { return *allocator_; }
            bool empty() const { return stackTop_ == stack_; }
            size_t getSize() const { return static_cast<size_t>(stackTop_ - stack_); }
            size_t getCapacity() const { return static_cast<size_t>(stackEnd_ - stack_); }
            
        private:
            template<typename T>
            void expand(size_t count) {
                // Only expand the capacity if the current stack exists. Otherwise just create a stack with initial capacity.
                size_t newCapacity;
                if (stack_ == 0)
                    newCapacity = initialCapacity_;
                else {
                    newCapacity = getCapacity();
                    newCapacity += (newCapacity + 1) / 2;
                }
                size_t newSize = getSize() + sizeof(T) * count;
                if (newCapacity < newSize)
                    newCapacity = newSize;
                
                resize(newCapacity);
            }
            
            void resize(size_t newCapacity) {
                const size_t size = getSize();  // Backup the current size
                stack_ = (char*)allocator_->realloc(stack_, getCapacity(), newCapacity);
                stackTop_ = stack_ + size;
                stackEnd_ = stack_ + newCapacity;
            }
            
            // Prohibit copy constructor & assignment operator.
            Stack(const Stack&);
            Stack& operator=(const Stack&);
            
            Allocator* allocator_;
            Allocator* ownAllocator;
            char *stack_;
            char *stackTop_;
            char *stackEnd_;
            size_t initialCapacity_;
        };
        
    } // namespace internal
} // namespace rapidjson

#endif // CSOUP_STACK_H_
