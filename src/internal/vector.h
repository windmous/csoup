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
        template <typename T, typename Allocator>
        class Vector {
        public:
            // Optimization note: Do not allocate memory for vector_ in constructor.
            // Do it lazily when first Push() -> Expand() -> Resize().
            Vector(Allocator* allocator = NULL, size_t vectorCapacity = 1) :
                                        data_(allocator, sizeof(T) * vectorCapacity),
                                        vectorSize_(0) {
                CSOUP_ASSERT(vectorCapacity > 0);
            }
            
            ~Vector() {
                if (0 != vectorSize_) {
                    T* p = data_.template bottom<T>();
                    for (size_t i = 0; i < vectorSize_; i ++, p ++) {
                        p->~T();
                    }
                }
            }
            
            void clear() {
                data_.clear();
                vectorSize_ = 0;
            }
            
            void shrinkToFit() {
                data_.shrinkToFit();
            }
            
            // Optimization note: try to minimize the size of this function for force inline.
            // Expansion is run very infrequently, so it is moved to another (probably non-inline) function.
            CSOUP_FORCEINLINE void push(const T& ele) {
                // Expand the stack if needed
                T* p = data_.template push<T>(1);
                new (p) T(ele);
                ++ vectorSize_;
            }
            
            CSOUP_FORCEINLINE T* push() {
                ++ vectorSize_;
                return data_.template push<T>(1);
            }
            
            T* pop() {
                CSOUP_ASSERT(vectorSize_ > 0);
                -- vectorSize_;
                return data_.template pop<T>(1);
            }
            
            void popAndDestroy() {
                pop()->~T();
            }
            
            T* top() const {
                CSOUP_ASSERT(vectorSize_ > 0);
                return data_.template top<T>();
            }
            
            T* get(size_t i) const {
                CSOUP_ASSERT(i < vectorSize_);
                return data_.template at<T>(i);
            }
            
            //Allocator& getAllocator() { return *allocator_; }
            bool empty() const { return vectorSize_ == 0; }
            size_t getSize() const { return vectorSize_; }
            size_t getCapacity() const { return data_.getCapacity() / sizeof(T); }
            
        private:
            // Prohibit copy constructor & assignment operator.
            Vector(const Vector&);
            Vector& operator=(const Vector&);
            
            size_t vectorSize_;
            ByteArray<Allocator> data_;
        };
        
    } // namespace internal
} // namespace rapidjson

#endif