#ifndef CSOUP_NODE_H_
#define CSOUP_NODE_H_

#include "../internal/nodedata.h"

namespace csoup {
    class Node {
    public:
        Node(NodeTypeEnum type, Node* parent, size_t siblingIndex, Allocator* allocator) : data_(type, parent, siblingIndex, allocator) {
            
        }
        virtual ~Node() = 0;
        
        size_t siblingIndex() const {
            return data_.siblingIndex_;
        }
        
        NodeTypeEnum type() const {
            return data_.type_;
        }
        
        Allocator* allocator() {
            return data_.allocator_;
        }
        
        Node* parentNode() {
            return data_.parent_;
        }
        
        const Node* parentNode() const {
            return data_.parent_;
        }
        
    protected:
        internal::NodeData& data() {
            return data_;
        }
        
        const internal::NodeData& data() const {
            return data_;
        }
        
        internal::ElementData& element() {
            return data_.v_.element_;
        }
        
        const internal::ElementData& element() const{
            return data_.v_.element_;
        }

//        internal::DocumentData& document() {
//            return data_.v_.doc_;
//        }
//        
//        const internal::DocumentData& document() const {
//            return data_.v_.doc_;
//        }
        
        internal::TextNodeData& text() {
            return data_.v_.text_;
        }
        
        const internal::TextNodeData& text() const {
            return data_.v_.text_;
        }

        void setSiblingIndex(size_t index) {
            data_.siblingIndex_ = index;
        }
        
        void setParentNode(Node* parent) {
            CSOUP_ASSERT(parent != NULL &&
                (parent->type() == CSOUP_NODE_ELEMENT || parent->type() == CSOUP_NODE_DOCUMENT));
            CSOUP_ASSERT(parentNode() == NULL);
            data_.parent_ = parent;
        }
        
        friend class Element;
    private:
        internal::NodeData data_;
    };
    
    inline Node::~Node() {
    }
}

#endif