#ifndef CSOUP_INTERNAL_VECTOR_H_
#define CSOUP_INTERNAL_VECTOR_H_
#include "bytearray.h"

namespace csoup {
    namespace internal {
        
        ///////////////////////////////////////////////////////////////////////////////
        // Vector
        
        //! A container used for storing elements of the same type
        /*! \tparam Allocator Allocator for allocating stack memory.
         */
        template <typename T>
        class Vector {
        public:
            // Optimization note: Do not allocate memory for vector_ in constructor.
            // Do it lazily when first Push() -> Expand() -> Resize().
            Vector(Allocator* allocator = NULL, size_t vectorCapacity = 1) :
                                    allocator_(allocator), ownAllocator(0), stack_(0),stackTop_(0), stackEnd_(0), initialCapacity_(vectorCapacity) {
                CSOUP_ASSERT(vectorCapacity > 0);
                if (!allocator_)
                    ownAllocator = allocator_ = new CrtAllocator();
            }
            
            ~Vector() {
                clear();
                allocator_->free(stack_);
                delete ownAllocator; // Only delete if it is owned by the stack
            }
            
            void clear() {
                if (0 != vectorSize_) {
                    T* p = static_cast<T*>(stack_);
                    for (size_t i = 0; i < vectorSize_; i ++, p ++) {
                        p->~T();
                    }
                }

                stack_ = stackTop_ = stackEnd_ = 0;
                vectorSize_ = 0;
            }
            
            void shrinkToFit() {
                if (empty()) {
                    // If the stack is empty, completely deallocate the memory.
                    Allocator::free(stack_);
                    stack_ = 0;
                    stackTop_ = 0;
                    stackEnd_ = 0;
                }
                else
                    resize(sizeof(T) * getSize());
            }
            
            // Optimization note: try to minimize the size of this function for force inline.
            // Expansion is run very infrequently, so it is moved to another (probably non-inline) function.
            CSOUP_FORCEINLINE void push(const T& obj) {
                // Expand the stack if needed
                ensureExtraSize(1);
                new (stackTop_ ++) T(obj);
            }
            
            CSOUP_FORCEINLINE T* push() {
                ensureExtraSize(1);
                return stackTop_ ++;
            }
            
            void pop() {
                CSOUP_ASSERT(!empty());
                (-- stackTop_)->~T();
            }
            
            const T* back() const {
                CSOUP_ASSERT(!empty());
                return stackTop_ - 1;
            }
            
            T* back() {
                CSOUP_ASSERT(!empty());
                return stackTop_ - 1;
            }
            
            T* at(size_t i) {
                CSOUP_ASSERT(i < getSize());
                return stack_ + i;
            }
            
            const T* at(size_t i) const {
                CSOUP_ASSERT(i < getSize());
                return stack_ + i;
            }
            
            void reserve(size_t n) {
                if (n > getSize()) ensureExtraSize(n - getSize());
            }
            
            void remove(size_t index) {
                CSOUP_ASSERT(index < getSize());
                at(index)->~T();
                std::memmove(stack_ + index, stack_ + index + 1,
                             sizeof(T) * (getSize() - index - 1));
                --stackTop_;
            }
            
            void insert(size_t index, const T& obj) {
                if (index > getSize()) index = getSize();
                ensureExtraSize(1);
                std::memmove(stack_ + index + 1, stack_ + index,
                             sizeof(T) * (getSize() - index));
                new (stack_ + index) T(obj);

                ++ stackTop_;
            }
            
            T* insert(size_t index) {
                if (index > getSize()) index = getSize();
                ensureExtraSize(1);
                std::memmove(stack_ + index + 1, stack_ + index,
                             sizeof(T) * (getSize() - index));
                
                ++ stackTop_;
                return stack_ + index;
            }
            
            //Allocator& getAllocator() { return *allocator_; }
            size_t getSize() const { return stackTop_ - stack_; }
            size_t getCapacity() const { return (stackEnd_ - stack_); }
            bool empty() const { return getSize() == 0; }
            
        private:
            // Prohibit copy constructor & assignment operator.
            Vector(const Vector&);
            Vector& operator=(const Vector&);
            
            void ensureExtraSize(size_t count) {
                // Expand the stack if needed
                if (stackTop_ + byteCount >= stackEnd_)
                    expand(byteCount);
            }
            
            void expand(size_t count) {
                // Only expand the capacity if the current stack exists. Otherwise just create a stack with initial capacity.
                size_t newCapacity;
                if (stack_ == 0)
                    newCapacity = initialCapacity_;
                else {
                    newCapacity = getCapacity();
                    newCapacity += (newCapacity + 1) / 2;
                }
                size_t newSize = getSize() + byteCount;
                if (newCapacity < newSize)
                    newCapacity = newSize;
                
                resize(newCapacity);
            }
            
            void resize(size_t count) {
                const size_t size = getSize();  // Backup the current size
                
                stack_ = (T*)allocator_->realloc(stack_, getCapacity() * sizeof(T),
                                                        count * sizeof(T));
                stackTop_ = stack_ + size;
                stackEnd_ = stack_ + count;
            }
            
            Allocator* allocator_;
            Allocator* ownAllocator;
            T *stack_;
            T *stackTop_;
            T *stackEnd_;
            size_t initialCapacity_;
        };
        
    } // namespace internal
} // namespace rapidjson

#endif