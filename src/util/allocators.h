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

#ifndef CSOUP_ALLOCATORS_H_
#define CSOUP_ALLOCATORS_H_

#include "common.h"

namespace csoup {

///////////////////////////////////////////////////////////////////////////////
// Allocator

/*! \class rapidjson::Allocator
    \brief Concept for allocating, resizing and freeing memory block.
    
    Note that Malloc() and Realloc() are non-static but Free() is static.
    
    So if an allocator need to support Free(), it needs to put its pointer in 
    the header of memory block.

\code
concept Allocator {
    static const bool kNeedFree;    //!< Whether this allocator needs to call Free().

    // Allocate a memory block.
    // \param size of the memory block in bytes.
    // \returns pointer to the memory block.
    void* Malloc(size_t size);

    // Resize a memory block.
    // \param originalPtr The pointer to current memory block. Null pointer is permitted.
    // \param originalSize The current size in bytes. (Design issue: since some allocator may not book-keep this, explicitly pass to it can save memory.)
    // \param newSize the new size in bytes.
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize);

    // Free a memory block.
    // \param pointer to the memory block. Null pointer is permitted.
    static void Free(void *ptr);
};
\endcode
*/
    class Allocator {
    public:
        Allocator(bool needFree) : flagNeedFree_(needFree) {
        }
        
        virtual ~Allocator() = 0;
        
        bool needFree() const {
            return flagNeedFree_;
        }
        
        virtual void* malloc(size_t size) = 0;
        virtual void free(const void* ptr) = 0;
        virtual void* realloc(void* ptr, size_t oriSize, size_t newSize) = 0;
        
        template <typename T>
        T* malloc_t() {
            return static_cast<T*>(malloc(sizeof(T)));
        }
        
        template <typename T>
        T* realloc_t(T* ptr, size_t oriSize, size_t newSize) {
            return static_cast<T*>(realloc(ptr, oriSize, newSize));
        }        
    private:
        bool flagNeedFree_;
    };
    
    inline Allocator::~Allocator() {}
    
    inline void* Allocator::malloc(size_t) {
        return 0;
    }
    
    inline void Allocator::free(const void* ptr) {}
    inline void* Allocator::realloc(void* ptr, size_t oriSize, size_t newSize) {
        return NULL;
    }
    
    
    class StumpAllocator : public Allocator {
    public:
        StumpAllocator() : Allocator(false) {}

        void* malloc(size_t size) { return NULL; }
        void* realloc(void* originalPtr, size_t originalSize, size_t newSize) {
            (void)originalSize;
            return NULL;
        }
        void free(const void *ptr) { (void)ptr; }
    };
    
    inline StumpAllocator* globalDumbAllocator() {
        static StumpAllocator GlobalDumbAllocator;
        return &GlobalDumbAllocator;
    }
    
///////////////////////////////////////////////////////////////////////////////
// CrtAllocator

//! C-runtime library allocator.
/*! This class is just wrapper for standard C library memory routines.
    \note implements Allocator concept
*/
    

    class CrtAllocator : public Allocator {
    public:
        CrtAllocator() : Allocator(kNeedFree) {}
        
        void* malloc(size_t size) { return std::malloc(size); }
        void* realloc(void* originalPtr, size_t originalSize, size_t newSize) {
            (void)originalSize;
            return std::realloc(originalPtr, newSize);
        }
        void free(const void *ptr) { std::free(const_cast<void*>(ptr)); }
    
    private:
        static const bool kNeedFree = true;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // MemoryPoolAllocator

    //! Default memory allocator used by the parser and DOM.
    /*! This allocator allocate memory blocks from pre-allocated memory chunks.

        It does not free memory blocks. And Realloc() only allocate new memory.

        The memory chunks are allocated by BaseAllocator, which is CrtAllocator by default.

        User may also supply a buffer as the first chunk.

        If the user-buffer is full then additional chunks are allocated by BaseAllocator.

        The user-buffer is not deallocated by this allocator.

        \tparam BaseAllocator the allocator type for allocating memory chunks. Default is   CrtAllocator.
        \note implements Allocator concept
     */
    class MemoryPoolAllocator : public Allocator {
    public:
        static const bool kNeedFree = false;    //!< Tell users that no need to call Free() with this allocator. (concept Allocator)

        //! Constructor with chunkSize.
        /*! \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
            \param baseAllocator The allocator for allocating memory chunks.
         */
        MemoryPoolAllocator(size_t chunkSize = kDefaultChunkCapacity, Allocator* baseAllocator = 0) :
            Allocator(kNeedFree), chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(0), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
        {
            if (!baseAllocator_)
                ownBaseAllocator_ = baseAllocator_ = new CrtAllocator();
            addChunk(chunk_capacity_);
        }

        //! Constructor with user-supplied buffer.
        /*! The user buffer will be used firstly. When it is full, memory pool allocates new chunk with chunk size.

            The user buffer will not be deallocated when this allocator is destructed.

            \param buffer User supplied buffer.
            \param size Size of the buffer in bytes. It must at least larger than sizeof(ChunkHeader).
            \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
            \param baseAllocator The allocator for allocating memory chunks.
         */
        MemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, Allocator* baseAllocator = 0) :
            Allocator(kNeedFree), chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(buffer), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
        {
            CSOUP_ASSERT(buffer != 0);
            CSOUP_ASSERT(size > sizeof(ChunkHeader));
            chunkHead_ = reinterpret_cast<ChunkHeader*>(buffer);
            chunkHead_->capacity = size - sizeof(ChunkHeader);
            chunkHead_->size = 0;
            chunkHead_->next = 0;
        }

        //! Destructor.
        /*! This deallocates all memory chunks, excluding the user-supplied buffer.
         */
        ~MemoryPoolAllocator() {
            clear();
            delete ownBaseAllocator_;
        }

        //! Deallocates all memory chunks, excluding the user-supplied buffer.
        void clear();

        //! Computes the total capacity of allocated memory chunks.
        /*! \return total capacity in bytes.
         */
        size_t capacity() const;

        //! Computes the memory blocks allocated.
        /*! \return total used bytes.
         */
        size_t size() const;

        //! Allocates a memory block. (concept Allocator)
        void* malloc(size_t size) {
            size = CSOUP_ALIGN(size);
            if (chunkHead_ == 0 || chunkHead_->size + size > chunkHead_->capacity)
                addChunk(chunk_capacity_ > size ? chunk_capacity_ : size);

            void *buffer = reinterpret_cast<char *>(chunkHead_ + 1) + chunkHead_->size;
            chunkHead_->size += size;
            return buffer;
        }

        //! Resizes a memory block (concept Allocator)
        void* realloc(void* originalPtr, size_t originalSize, size_t newSize);
        
        //! Frees a memory block (concept Allocator)
        void free(const void *ptr) {
            Allocator::free(ptr);
        } // Do nothing

    private:
        //! Copy constructor is not permitted.
        MemoryPoolAllocator(const MemoryPoolAllocator& rhs) /* = delete */;
        //! Copy assignment operator is not permitted.
        MemoryPoolAllocator& operator=(const MemoryPoolAllocator& rhs) /* = delete */;
        
        //! Creates a new chunk.
        /*! \param capacity Capacity of the chunk in bytes.
         */
        void addChunk(size_t capacity) {
            ChunkHeader* chunk = reinterpret_cast<ChunkHeader*>(baseAllocator_->malloc(sizeof(ChunkHeader) + capacity));
            chunk->capacity = capacity;
            chunk->size = 0;
            chunk->next = chunkHead_;
            chunkHead_ =  chunk;
        }

        static const int kDefaultChunkCapacity = 64 * 1024; //!< Default chunk capacity.

        //! Chunk header for perpending to each chunk.
        /*! Chunks are stored as a singly linked list.
         */
        struct ChunkHeader {
            size_t capacity;    //!< Capacity of the chunk in bytes (excluding the header itself).
            size_t size;        //!< Current size of allocated memory in bytes.
            ChunkHeader *next;  //!< Next chunk in the linked list.
        };

        ChunkHeader *chunkHead_;    //!< Head of the chunk linked-list. Only the head chunk serves allocation.
        size_t chunk_capacity_;     //!< The minimum capacity of chunk when they are allocated.
        void *userBuffer_;          //!< User supplied buffer.
        Allocator* baseAllocator_;  //!< base allocator for allocating memory chunks.
        Allocator* ownBaseAllocator_;   //!< base allocator created by this object.
    };

} // namespace csoup

#endif // CSOUP_ALLOCATORS_H_
