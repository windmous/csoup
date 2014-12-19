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
    template <typename T>
    class SharedPtr;
    template <typename T>
    class WeakPtr;
    
    class SharedPtrControlBlock {
        template <typename T> friend class SharedPtr;
        template <typename T> friend class WeakPtr;
    private:
        SharedPtrControlBlock(Allocator* allocator) : refCount_(1), weakCount_(1), allocator_(allocator) { }
        int refCount_;
        int weakCount_;
        Allocator* allocator_;
    };
    
    // NOTE: It is strongly encouraged to use scoped_ptr if you could.
    //       shared_ptr should be only used at the cases that
    //       there is no clear owner for the object, anyone reference the object
    //       may need take care if the object should be deleted.
    // NOTE: Don't use this class to replace boost::shared_ptr when working with
    //       thrift.
    template <typename T>
    class SharedPtr {
        template <typename U> friend class WeakPtr;
    public:
        typedef T element_type;
        
        explicit SharedPtr(T* ptr, Allocator* allocator)
        : ptr_(ptr), controlBlock_(NULL) {
            if (ptr != NULL) {
                controlBlock_ = CSOUP_NEW1(allocator, SharedPtrControlBlock, allocator);
            }
        }
        
        // Copy constructor: makes this object a copy of ptr
        template <typename U>
        SharedPtr(const SharedPtr<U>& ptr)  // NOLINT
        : ptr_(NULL), controlBlock_(NULL) {
            initialize(ptr);
        }
        // Need non-templated version to prevent the compiler-generated default
        SharedPtr(const SharedPtr<T>& ptr)  // NOLINT
        : ptr_(NULL), controlBlock_(NULL) {
            initialize(ptr);
        }
        
        // Assignment operator. Replaces the existing shared_ptr with ptr.
        template <typename U>
        SharedPtr<T>& operator = (const SharedPtr<U>& ptr) {
            if (ptr_ != ptr.ptr_) {
                SharedPtr<T> me(ptr);   // will hold our previous state to be destroyed.
                swap(me);
            }
            return *this;
        }
        
        // Need non-templated version to prevent the compiler-generated default
        SharedPtr<T>& operator = (const SharedPtr<T>& ptr) {
            if (ptr_ != ptr.ptr_) {
                SharedPtr<T> me(ptr);   // will hold our previous state to be destroyed.
                swap(me);
            }
            return *this;
        }
        
        ~SharedPtr() {
            if (ptr_ != NULL) {
                if ( -- controlBlock_->refCount_ == 0) {
                    ptr_->~T();
                    controlBlock_->allocator_->free(ptr_);
                    
                    // weak_count_ is defined as the number of weak_ptrs that observe
                    // ptr_, plus 1 if refcount_ is nonzero.
                    if (-- controlBlock_->weakCount_ == 0) {
                        controlBlock_->allocator_->free(controlBlock_);
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
            std::swap(controlBlock_, r.controlBlock_ );
        }
        
        // The following function is useful for gaining access to the underlying
        // pointer when a shared_ptr remains in scope so the reference-count is
        // known to be > 0 (e.g. for parameter passing).
        T* get() const {
            return ptr_;
        }
        
        const T& operator*() const {
            return *ptr_;
        }
        
        T& operator*() {
            return *ptr_;
        }
        
        const T* operator->() const {
            return ptr_;
        }
        
        T* operator-> () {
            return ptr_;
        }
        
        int useCount() const {
            return controlBlock_ ? controlBlock_->refCount_ : 1;
        }
        
        bool unique() const {
            return useCount() == 1;
        }
        
    private:
        // If r is non-empty, initialize *this to share ownership with r,
        // increasing the underlying reference count.
        // If r is empty, *this remains empty.
        // Requires: this is empty, namely this->ptr_ == NULL.
        template <typename U>
        void initialize(const SharedPtr<U>& r) {
            if (r.controlBlock_ != NULL) {
                ++ r.controlBlock_->refCount_;
                
                ptr_ = r.ptr_;
                controlBlock_ = r.controlBlock_;
            }
        }
        
        T* ptr_;
        SharedPtrControlBlock* controlBlock_;
        
        template <typename U>
        friend class shared_ptr;
    };
    
    // Matches the interface of std::swap as an aid to generic programming.
    template <typename T> void swap(SharedPtr<T>& r, SharedPtr<T>& s) {
        r.swap(s);
    }
    
    // Weak ptrs:
    //   The weak_ptr auxiliary class is used to break ownership cycles. A weak_ptr
    //   points to an object that's owned by a shared_ptr, but the weak_ptr is an
    //   observer, not an owner. When the last shared_ptr that points to the object
    //   disappear, the weak_ptr expires, at which point the expired() member
    //   function will return true.
    //   You can't directly get a raw pointer from weak_ptr, to access a weak_ptr's
    //   pointed-to object, use lock() to obtain a temporary shared_ptr.
    //   See the draft C++ standard (as of October 2007 the latest draft is N2461)
    //   for the detailed specification.
    template <typename T>
    class WeakPtr {
        template <typename U> friend class WeakPtr;
    public:
        typedef T element_type;
        
        // Create an empty (i.e. already expired) weak_ptr.
        WeakPtr() : ptr_(NULL), controlBlock_(NULL) { }
        
        // Create a weak_ptr that observes the same object that ptr points
        // to.  Note that there is no race condition here: we know that the
        // control block can't disappear while we're looking at it because
        // it is owned by at least one shared_ptr, ptr.
        template <typename U> WeakPtr(const SharedPtr<U>& ptr) {
            copyFrom(ptr.ptr_, ptr.control_block_);
        }
        
        // Copy a weak_ptr. The object it points to might disappear, but we
        // don't care: we're only working with the control block, and it can't
        // disappear while we're looking at because it's owned by at least one
        // weak_ptr, ptr.
        template <typename U> WeakPtr(const WeakPtr<U>& ptr) {
            copyFrom(ptr.ptr_, ptr.control_block_);
        }
        
        // Need non-templated version to prevent default copy constructor
        WeakPtr(const WeakPtr& ptr) {
            copyFrom(ptr.ptr_, ptr.control_block_);
        }
        
        // Destroy the weak_ptr. If no shared_ptr owns the control block, and if
        // we are the last weak_ptr to own it, then it can be deleted. Note that
        // weak_count_ is defined as the number of weak_ptrs sharing this control
        // block, plus 1 if there are any shared_ptrs. We therefore know that it's
        // safe to delete the control block when weak_count_ reaches 0, without
        // having to perform any additional tests.
        ~WeakPtr() {
            if (controlBlock_ != NULL &&
                (-- controlBlock_->weakCount_ == 0)) {
                controlBlock_->allocator_->free(controlBlock_);
            }
        }
        
        WeakPtr& operator=(const WeakPtr& ptr) {
            if (&ptr != this) {
                WeakPtr tmp(ptr);
                tmp.swap(*this);
            }
            return *this;
        }
        template <typename U> WeakPtr& operator=(const WeakPtr<U>& ptr) {
            WeakPtr tmp(ptr);
            tmp.swap(*this);
            return *this;
        }
        template <typename U> WeakPtr& operator=(const SharedPtr<U>& ptr) {
            WeakPtr tmp(ptr);
            tmp.swap(*this);
            return *this;
        }
        
        void swap(WeakPtr& ptr) {
            std::swap(ptr_, ptr.ptr_);
            std::swap(controlBlock_, ptr.controlBlock_);
        }
        
        void reset() {
            WeakPtr tmp;
            tmp.swap(*this);
        }
        
        // Return the number of shared_ptrs that own the object we are observing.
        // Note that this number can be 0 (if this pointer has expired).
        int useCount() const {
            return controlBlock_ != NULL ? controlBlock_->refCount_ : 0;
        }
        
        bool expired() const { return useCount() == 0; }
        
    private:
        void copyFrom(T* ptr, SharedPtrControlBlock* controlblock) {
            ptr_ = ptr;
            controlBlock_ = controlblock;
            if (controlBlock_ != NULL)
                ++ controlBlock_->weakCount_;
        }
        
    private:
        element_type* ptr_;
        SharedPtrControlBlock* controlBlock_;
    };
    
    template <typename T> void swap(WeakPtr<T>& r, WeakPtr<T>& s) {
        r.swap(s);
    }
}


#endif // CSOUP_SHARED_PTR_H_
