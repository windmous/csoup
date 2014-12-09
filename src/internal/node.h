#ifndef CSOUP_NODE_H_
#define CSOUP_NODE_H_

#include "../utils/common.h"
#include "../internl/vector"
#include "attributes.h"


namespace csoup {
    template <typename CharType, typename Allocator>
    class Node {
    public:
        Node(Attributes* attributes, Allocator* allocator) :
            allocator_(allocator), parentNode_(NULL), siblingIndex_(), \
            attributes_(attributes), childNodes_(NULL) {
                
            CSOUP_ASSERT(allocator != NULL);
                
            //childNodes_ = allocator->malloc_t< Vecotor<Node*> >();
            //new (childNodes_) Vector<Node*>(allocator, 4);
        }
        
        virtual ~Node() {
            if (attributes_) {
                attributes_->~Attributes();
                allocator_->free(attributes_);
                attributes_ = NULL;
            }
            
            if (childNodes_) {
                childNodes_->~Vector<Node*>();
                allocator_->free(childNodes_);
                childNodes_ = NULL;
            }
        }
        
        
        //////////////////////////////////////////////////
        // Methods about attributes
        
        StringRef attr(const StringRef& key) const {
            return attr(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        StringRef attr(AttributeNamespaceEnum space, const StringRef& key) const {
            return attributes_ ? attributes_->get(key) : StringRef("");
        }
        
        const Attributes* attributes() const {
            ensureAttributes();
            return attributes_;
        }
        
        bool addAttribute(const StringRef& key, const StringRef& value) {
            addAttribute(CSOUP_ATTR_NAMESPACE_NONE, key, value);
        }
        
        bool addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value) {
            ensureAttributes();
            return attributes_->addAttribute(space, key, value);
        }
        
        bool hasAttribute(const StringRef& key) const {
            return hasAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        bool hasAttribute(AttributeNamespaceEnum space, const StringRef& key) {
            return attributes_ ? attributes_->hasAttribute(space, key) : false;
        }

        void removeAttribute(const StringRef& key) {
            return removeAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        void removeAttribute(AttributeNamespaceEnum space, const StringRef& key) {
            if (attributes_) attributes_->removeAttribute(space, key);
        }
        
        ////////////////////////////////////////////////
        // Methods about children node
        size_t childNodeSize() const {
            return childNodes_ ? childNodes_->size() : 0;
        }
        
        Node* childNode(size_t index) const {
            CSOUP_ASSERT(index < childNodeSize());
            return childNodes_->at(index);
        }
        
        size_t childNodesAsArray(Node** arrayBuffer, size_t arraySize) const {
            const size_t countOfChildren = childNodeSize();
            
            if (countOfChildren == 0 || arraySize < countOfChildren) {
                return countOfChildren;
            }
            
            for (size_t i < 0; i < countOfChildren; ++ i) {
                arrayBuffer[i] = childNodes_[i];
            }
            
            return countOfChildren;
        }
        
        void addChild(Node* )
    
        
        ////////////////////////////////////////////////
        // Methods about this node
        
        // For the reason that internal::Vecotr is just for internal use,
        // we should never make Vector object exposed to the user.
        
        Node* parent() const {
            return parentNode_;
        }
        
        size_t siblingIndex() const {
            return siblingIndex_;
        }
        
    private:
        void ensureAttributes() {
            if (!attributes_) {
                attributes_ = malloc_t< Attributes > >();
                new (attributes_) (allocator_);
            }
        }
        
        void ensureChildNodes() {
            if (!childNodes_) {
                childNodes_ = allocator->malloc_t< Vecotor<Node*> >();
                new (childNodes_) (allocator_, 4);
            }
        }
        
        void setSiblingIndex(size_t index) {
            CSOUP_ASSERT(parentNode_ != NULL && index < parentNode_->childNodeSize());
            siblingIndex_ = index;
        }
        
        void reindexChildren() {
            // we can ignore whether childNodes_ is NULL or not;
            const size_t countOfChildren = childNodeSize();
            
            for (size_t i = 0; i < countOfChildren; ++ i) {
                childNodes_->setSiblingIndex(i);
            }
        }
        
        Allocator* allocator_;
        Node* parentNode_;
        size_t siblingIndex_;
        Attributes* attributes_;
        internal::Vector<Node*>* childNodes_;
    };
}

#endif // CSOUP_NODE_H_
