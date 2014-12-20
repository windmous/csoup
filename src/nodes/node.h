#ifndef CSOUP_NODE_H_
#define CSOUP_NODE_H_

#include "../internal/nodedata.h"
#include "../util/stringref.h"

namespace csoup {
    class Document;
    class Element;
    
    class Node {
    public:
        Node(NodeTypeEnum type, Node* parent, size_t siblingIndex, const StringRef& baseUri, Allocator* allocator)
        : type_(type), parent_(parent), siblingIndex_(siblingIndex), baseUri_(NULL), allocator_(allocator) {
            CSOUP_ASSERT(allocator != NULL);
            baseUri_ = CSOUP_NEW2(allocator, String, baseUri, allocator);
        }
        
        virtual ~Node() = 0;
        
        size_t siblingIndex() const {
            return siblingIndex_;
        }
        
        NodeTypeEnum type() const {
            return type_;
        }
        
        Allocator* allocator() {
            return allocator_;
        }
        
        Element* parentNode();
        
        const Node* parentNode() const {
            return parent_;
        }
        
        StringRef baseUri() const {
            return baseUri_ ? baseUri_->ref() : StringRef("");
        }
        
        void before(Node* node);
        void after(Node* node);
        
        // Gets the Document associated with this Node.
        Document* ownerDocument() const;
        
        void removeFromParent(bool del);
        
    protected:
        void setSiblingIndex(size_t index) {
            siblingIndex_ = index;
        }
        
        void setParentNode(Node* parent);
        
        friend class Element;
        
        NodeTypeEnum type_;
        
        // This is a weak reference to parent node; Don't try to release this node;
        Node* parent_;
        size_t siblingIndex_;
        
        String* baseUri_;
        Allocator* allocator_;
    };
    
    inline Node::~Node() {
        if (baseUri_) {
            CSOUP_DELETE(allocator(), baseUri_);
        }
    }
}

#endif