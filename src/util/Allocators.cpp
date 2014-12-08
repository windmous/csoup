#include "allocators.h"

namespace csoup {
    void MemoryPoolAllocator::clear() {
        while(chunkHead_ != 0 && chunkHead_ != userBuffer_) {
            ChunkHeader* next = chunkHead_->next;
            baseAllocator_->free(chunkHead_);
            chunkHead_ = next;
        }
    }
    
    size_t MemoryPoolAllocator::size() const {
        size_t size = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            size += c->size;
        return size;
    }
    
    size_t MemoryPoolAllocator::capacity() const {
        size_t capacity = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            capacity += c->capacity;
        return capacity;
    }
    
    void* MemoryPoolAllocator::realloc(void *originalPtr, size_t originalSize, size_t newSize) {
        if (originalPtr == 0)
            return malloc(newSize);
        
        // Do not shrink if new size is smaller than original
        if (originalSize >= newSize)
            return originalPtr;
        
        // Simply expand it if it is the last allocation and there is sufficient space
        if (originalPtr == (char *)(chunkHead_ + 1) + chunkHead_->size - originalSize) {
            size_t increment = static_cast<size_t>(newSize - originalSize);
            increment = CSOUP_ALIGN(increment);
            if (chunkHead_->size + increment <= chunkHead_->capacity) {
                chunkHead_->size += increment;
                return originalPtr;
            }
        }
        
        // Realloc process: allocate and copy memory, do not free original buffer.
        void* newBuffer = malloc(newSize);
        CSOUP_ASSERT(newBuffer != 0);   // Do not handle out-of-memory explicitly.
        return std::memcpy(newBuffer, originalPtr, originalSize);
    }
}