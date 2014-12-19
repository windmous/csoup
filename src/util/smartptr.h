//
//  sharedptr.h
//  csoup
//
//  Created by mac on 12/18/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_SHARED_PTR_H_
#define CSOUP_SHARED_PTR_H_
#include "allocators.h"
#include <algorithm>

namespace csoup {
    // This class is an internal implementation detail for shared_ptr.
    class SharedPtrControlBlock {
        template <typename T> friend class shared_ptr;
        template <typename T> friend class weak_ptr;
    private:
        SharedPtrControlBlock() : refcount_(1), weak_count_(1) { }
        int refcount_;
        int weak_count_;
    };
    
    // NOTE: It is strongly encouraged to use scoped_ptr if you could.
    //       shared_ptr should be only used at the cases that
    //       there is no clear owner for the object, anyone reference the object
    //       may need take care if the object should be deleted.
    // NOTE: Don't use this class to replace boost::shared_ptr when working with
    //       thrift.
    template <typename T>
    class SharedPtr {
        template <typename U> friend class weak_ptr;
    public:
        typedef T element_type;
        
        explicit SharedPtr(T* ptr = NULL)
        : ptr_(ptr),
        control_block_(ptr != NULL ? new SharedPtrControlBlock : NULL) {
        }
        
        // Copy constructor: makes this object a copy of ptr
        template <typename U>
        SharedPtr(const SharedPtr<U>& ptr)  // NOLINT
        : ptr_(NULL),
        control_block_(NULL) {
            Initialize(ptr);
        }
        // Need non-templated version to prevent the compiler-generated default
        SharedPtr(const SharedPtr<T>& ptr)  // NOLINT
        : ptr_(NULL),
        control_block_(NULL) {
            Initialize(ptr);
        }
        
        // Assignment operator. Replaces the existing shared_ptr with ptr.
        template <typename U>
        SharedPtr<T>& operator=(const SharedPtr<U>& ptr) {
            if (ptr_ != ptr.ptr_) {
                SharedPtr<T> me(ptr);   // will hold our previous state to be destroyed.
                swap(me);
            }
            return *this;
        }
        
        // Need non-templated version to prevent the compiler-generated default
        SharedPtr<T>& operator=(const SharedPtr<T>& ptr) {
            if (ptr_ != ptr.ptr_) {
                SharedPtr<T> me(ptr);   // will hold our previous state to be destroyed.
                swap(me);
            }
            return *this;
        }
        
        ~SharedPtr() {
            if (ptr_ != NULL) {
                if (AtomicDecrement(&control_block_->refcount_) == 0) {
                    delete ptr_;
                    
                    // weak_count_ is defined as the number of weak_ptrs that observe
                    // ptr_, plus 1 if refcount_ is nonzero.
                    if (AtomicDecrement(&control_block_->weak_count_) == 0) {
                        delete control_block_;
                    }
                }
            }
        }
        
        // Replaces underlying raw pointer with the one passed in.  The reference
        // count is set to one (or zero if the pointer is NULL) for the pointer
        // being passed in and decremented for the one being replaced.
        void reset(T* p = NULL) {
            if (p != ptr_) {
                SharedPtr<T> tmp(p);
                tmp.swap(*this);
            }
        }
        
        // Exchanges the contents of this with the contents of r.  This function
        // supports more efficient swapping since it eliminates the need for a
        // temporary shared_ptr object.
        void swap(SharedPtr<T>& r) {
            std::swap(ptr_, r.ptr_);
            std::swap(control_block_, r.control_block_);
        }
        
        // The following function is useful for gaining access to the underlying
        // pointer when a shared_ptr remains in scope so the reference-count is
        // known to be > 0 (e.g. for parameter passing).
        T* get() const {
            return ptr_;
        }
        
        T& operator*() const {
            return *ptr_;
        }
        
        T* operator->() const {
            return ptr_;
        }
        
        int use_count() const {
            return control_block_ ? control_block_->refcount_ : 1;
        }
        
        bool unique() const {
            return use_count() == 1;
        }
        
    private:
        // If r is non-empty, initialize *this to share ownership with r,
        // increasing the underlying reference count.
        // If r is empty, *this remains empty.
        // Requires: this is empty, namely this->ptr_ == NULL.
        template <typename U>
        void Initialize(const SharedPtr<U>& r) {
            if (r.control_block_ != NULL) {
                AtomicIncrement(&r.control_block_->refcount_);
                
                ptr_ = r.ptr_;
                control_block_ = r.control_block_;
            }
        }
        
        T* ptr_;
        SharedPtrControlBlock* control_block_;
        
        template <typename U>
        friend class shared_ptr;
    };
    
    // Matches the interface of std::swap as an aid to generic programming.
    template <typename T> void swap(SharedPtr<T>& r, SharedPtr<T>& s) {
        r.swap(s);
    }
}
#endif // CSOUP_SHARED_PTR_H_
