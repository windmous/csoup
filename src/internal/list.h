#ifndef CSOUP_INTERNAL_LIST_H_
#define CSOUP_INTERNAL_LIST_H_

#include "../util/common.h"

namespace csoup {
    namespace internal {
        template <typename T>
        class List {
        public:
            List(Allocator* allocator = 0) : allocator_(allocator), ownAllocator(0), listHead_(0), listTail_(0), size_(0){
                
                if (!allocator_)
                    ownAllocator = allocator_ = new CrtAllocator();
            }
            
            ~List() {
                clear();
                delete ownAllocator;
            }
            
            size_t getSize() const {
                return size_;
            }
            
            bool empty() const {
                return getSize() == 0;
            }
            
            const T* at(size_t index) const {
                CSOUP_ASSERT(index < size_);
                if (index == size_ - 1) return *listTail_->data_;
                
                ListNode* p = listHead_;
                for (size_t i = 0; i < index; ++ i, p = p->next_)
                    ;
                
                return p->data_;
            }
            
            T* at(size_t index) {
                return const_cast<T*>(static_cast< const List<T>& >(*this).get(index));
            }
            
            void push(const T& obj) {
                pushImpl(createNode(obj));
            }
            
            T* push() {
                ListNode* p = createNode();
                pushImpl(p);
                return p->data_;
            }
            
            void pop() {
                CSOUP_ASSERT(listTail_);
                remove(size_ - 1);
            }
            
            T* back() {
                CSOUP_ASSERT(listTail_);
                return listTail_->data_;
            }
            
            const T* back() const {
                CSOUP_ASSERT(listTail_);
                return listTail_->data_;
            }
            
            void insert(size_t index, const T& obj) {
                insertImpl(index, createNode(obj));
            }
            
            T* insert(size_t index) {
                ListNode* ret = createNode();
                insertImpl(index, ret);
                return ret;
            }
            
            
            void remove(size_t index) {
                CSOUP_ASSERT(index < size_);
                
                ListNode* del;
                if (index == size_ - 1) {
                    del = listTail_;
                } else {
                    del = listHead_;
                    for (size_t i = 0; i < index; ++ i, del = del->next_)
                        ;
                }
                
                if (del == listTail_) listTail_ = listTail_->prev_;
                if (del == listHead_) listHead_ = listHead_->next_;
                if (del->prev_) del->prev_->next_ = del->next_;
                if (del->next_) del->next_->prev_ = del->prev_;
                
                destroyNode(del);
                -- size_;
            }
            
            void clear() {
                while (listHead_) {
                    ListNode* p = listHead_;
                    listHead_ = listHead_->next_;
                    
                    destroyNode(p);
                }
                
                listHead_ = listTail_ = 0;
            }
            
        private:
            struct ListNode {
                ListNode() : data_(0), next_(0), prev_(0) {
                }
                
                T* data_;
                ListNode* next_;
                ListNode* prev_;
            };
            
            ListNode* createNode(const T& obj) {
                ListNode* ret = allocator_->malloc(sizeof(ListNode));
                ret->data_ = allocator_->malloc(sizeof(ListNode));
                
                new (ret) ListNode();
                new (ret->data_) T(obj);
                ++ size_;
                
                return ret;
            }
            
            ListNode* createNode() {
                ListNode* ret = allocator_->malloc(sizeof(ListNode));
                ret->data_ = allocator_->malloc(sizeof(ListNode));
                
                new (ret) ListNode();
                ++ size_;
                
                return ret;
            }
            
            void destroyNode(ListNode* node) {
                if (node->data_) node->data_->~T();
                allocator_->free(node->data_);
                allocator_->free(node);
                -- size_;
            }
            
            void pushImpl(ListNode* node) {
                node->prev_ = listTail_;
                
                if (!listHead_) {
                    listHead_ = node;
                } else {
                    listTail_->next_ = node;
                }
                
                listTail_ = node;
            }
            
            void insertImpl(size_t index, ListNode* node) {
                if (index >= size_) pushImpl(node);
                else {
                    ListNode* p = listHead_;
                    for (size_t i = 0; i < index; ++ i, p = p->next_)
                        ;
                    
                    node->next_ = p;
                    node->prev_ = p->prev_;
                    if (p->prev_) p->prev_->next_ = node;
                    p->prev_ = node;
                    
                    if (p == listHead_) listHead_ = node;
                }
            }
            
            Allocator* allocator_;
            Allocator* ownAllocator;
            ListNode *listHead_;
            ListNode *listTail_;
            size_t size_;
        };
    }
}

#endif