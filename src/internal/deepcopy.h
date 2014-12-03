#ifndef CSOUP_INTERNAL_DEEPCOPY_H_
#define CSOUP_INTERNAL_DEEPCOPY_H_

namespace csoup {
    namespace internal {
        template <typename T, typename Allocator>
        T deepcopy(const T& obj, Allocator* allocator) {
            return obj;
        }
        
        template <typename T, typename Allocator>
        void destroy(T* obj, Allocator* allocator) {
            obj->~T();
        }
    }
}

#endif