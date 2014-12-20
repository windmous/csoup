#ifndef CSOUP_ELEMENT_H_
#define CSOUP_ELEMENT_H_

#include "../internal/nodedata.h"
#include "allocators.h"
#include "attributes.h"
#include "node.h"
#include "textnode.h"
#include "datanode.h"
#include "comment.h"
#include "tag.h"

namespace csoup {
    class ElementsRef;
    
    class Element : public Node {
    public:
        Element(const StringRef& tagName, const Attributes& attributes, const StringRef& baseUri, Allocator* allocator) :
                Node(CSOUP_NODE_ELEMENT, NULL, 0, baseUri, allocator) {
            CSOUP_ASSERT(Tag::isKnownTag(tagName));
                    
            tag_ = Tag::valueOf(tagName);
            attributes_ = new (allocator->malloc_t<Attributes>()) Attributes(attributes, allocator);
            childNodes_ =  NULL;
            classes_ = NULL;
        }
        
        Element(const StringRef& tagName, const StringRef& baseUri, Allocator* allocator) :
        Node(CSOUP_NODE_ELEMENT, NULL, 0, baseUri, allocator) {
            CSOUP_ASSERT(Tag::isKnownTag(tagName));
            
            tag_ = Tag::valueOf(tagName);
            attributes_ = NULL;
            childNodes_ =  NULL;
            classes_ = NULL;
        }

        ~Element() {
            for (size_t i = 0; i < childNodes_->size(); ++ i) {
                CSOUP_DELETE(allocator(), (*childNodes_->at(i)));
            }
            CSOUP_DELETE(allocator(), attributes_);
            CSOUP_DELETE(allocator(), childNodes_);
            CSOUP_DELETE(allocator(), classes_);
        }
        
        //////////////////////////////////////////////////
        // Methods about element
        
        Tag* tag() const {
            return tag_;
        }
        
        StringRef tagName() const {
            return tag_->tagName();
        }
        
        void setTagName(const StringRef& tagName) {
            tag_ = Tag::valueOf(tagName);
        }
        
        /////////////////////////////////////////////////
        // Methods about siblings
        Node* previousSibling() {
            Node* par = parentNode();
            if (par == NULL || par->siblingIndex() == 0) {
                return NULL;
            }
            
            CSOUP_ASSERT(par->type() == CSOUP_NODE_ELEMENT || CSOUP_NODE_DOCUMENT);
            return static_cast<Element*>(par)->childNode(siblingIndex() - 1);
        }
        
        Node* nextSibling() {
            Node* par = parentNode();
            if (par == NULL) {
                return NULL;
            }
            
            CSOUP_ASSERT(par->type() == CSOUP_NODE_ELEMENT || CSOUP_NODE_DOCUMENT);
            
            Element* elementPar = static_cast<Element*>(par);
            size_t siblingCount = elementPar->childNodeSize();
            return (siblingIndex() + 1 < siblingCount) ?
                            elementPar->childNode(siblingIndex() + 1) : NULL;
        }
        
        void parents(ElementsRef* output) {
            accumulateParents(this, output);
        }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //////////////////////////////////////////////////
        // Methods about attributes
        
        StringRef attr(const StringRef& key) const {
            return attr(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        StringRef attr(AttributeNamespaceEnum space, const StringRef& key) const {
            return attributes_ ? attributes_->get(key) : StringRef("");
        }
        
        const Attributes* attributes() const {
            return attributes_;
        }
        
        void addAttribute(const StringRef& key, const StringRef& value) {
            addAttribute(CSOUP_ATTR_NAMESPACE_NONE, key, value);
        }
        
        void addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value) {
            ensureAttributes()->addAttribute(space, key, value);
        }
        
        void addAttributes(const Attributes& attrs) {
            ensureAttributes()->addAttributes(attrs);
        }
        
        bool hasAttribute(const StringRef& key) const {
            return hasAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        bool hasAttribute(AttributeNamespaceEnum space, const StringRef& key) const {
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
        
        const Node* childNode(size_t index) const {
            CSOUP_ASSERT(index < childNodeSize());
            return *childNodes_->at(index);
        }
        
        Node* childNode(size_t index) {
            CSOUP_ASSERT(index < childNodeSize());
            return *childNodes_->at(index);
        }
        
        void removeChild(size_t index, bool del) {
            CSOUP_ASSERT(index < childNodeSize());
            
            // the node in vector would be destroyed
            if (del) {
                CSOUP_DELETE(allocator(), *childNodes_->at(index));
            }
            
            childNodes_->remove(index);
            reindexChildren();
        }
        
        void removeChild(Node* node, bool del) {
            CSOUP_ASSERT(node->parentNode() == this);
            removeChild(node->siblingIndex(), del);
        }
        
        ///////////////////////////////////////////////
        // !!!!!!!!!!!!!!!!
        //template <NodeTypeEnum>
        //Node* addChild(NodeTypeEnum type);
        
        //void addChild(size_t index, Node* node) {
        //    CSOUP_ASSERT(node->parentNode() == NULL);
        //    ensureChildNodes();
        //}
        //Node*`
        
        size_t childNodesAsArray(const Node** arrayBuffer, size_t arraySize) const {
            const size_t countOfChildren = childNodeSize();
            
            if (countOfChildren == 0 || arraySize < countOfChildren) {
                return countOfChildren;
            }
            
            for (size_t i = 0; i < countOfChildren; ++ i) {
                arrayBuffer[i] = *(childNodes_->at(i));
            }
            
            return countOfChildren;
        }
        
        void insertNode(size_t index, Node* node) {
            *insert(index) = node;
            reindexChildren(index);
        }
        
        void appendNode(Node* node) {
            *append() = node;
            reindexChildren(childNodes_->size() - 1);
        }
        
        Element* insertElement(size_t index, const StringRef& tagName, const Attributes& attributes) {
            Element* ret = new (allocator()->malloc_t<Element>()) Element(tagName, attributes, baseUri(), allocator());
            ret->setParentNode(this);
            
            childNodes_->insert(index, ret);
            reindexChildren(index);
            
            return ret;
        }
        
        Element* insertElement(size_t index, const StringRef& tagName) {
            Element* ret = new (allocator()->malloc_t<Element>()) Element(tagName, baseUri(), allocator());
            ret->setParentNode(this);
            
            childNodes_->insert(index, ret);
            reindexChildren(index);
            
            return ret;
        }
        
        Element* appendElement(const StringRef& tagName, const Attributes& attributes) {
            Element* ret = new (allocator()->malloc_t<Element>()) Element(tagName, attributes, baseUri(), allocator());
            ret->setParentNode(this);
            
            childNodes_->push(ret);
            ret->setSiblingIndex(childNodeSize() - 1);
            
            return ret;
        }
        
        Element* appendElement(const StringRef& tagName) {
            Element* ret = new (allocator()->malloc_t<Element>()) Element(tagName, baseUri(), allocator());
            ret->setParentNode(this);
            
            childNodes_->push(ret);
            ret->setSiblingIndex(childNodeSize() - 1);
            
            return ret;
        }
        
#define CREATE_TEXT_BASED_NODE_METHOD(NodeTypeName) \
    NodeTypeName* insert##NodeTypeName(size_t index, const StringRef& text) {\
        NodeTypeName* ret = allocator()->malloc_t<NodeTypeName>(); \
        new (ret) NodeTypeName(text, baseUri(), allocator()); \
        ret->setParentNode(this); \
        childNodes_->insert(index, ret); \
        reindexChildren(index); \
        return ret; \
    } \
    \
    NodeTypeName* append##NodeTypeName(size_t index, const StringRef& text) { \
        NodeTypeName* ret = allocator()->malloc_t<NodeTypeName>(); \
        new (ret) NodeTypeName(text, baseUri(), allocator()); \
        ret->setParentNode(this); \
        childNodes_->push(ret); \
        ret->setSiblingIndex(childNodeSize() - 1); \
        return ret; \
    }
        
    CREATE_TEXT_BASED_NODE_METHOD(DataNode)
    CREATE_TEXT_BASED_NODE_METHOD(CommentNode)
    CREATE_TEXT_BASED_NODE_METHOD(TextNode)
        
    protected:
        Element(NodeTypeEnum nodeType, const StringRef& tagName, const StringRef& baseUri, Allocator* allocator) :
        Node(nodeType, NULL, 0, baseUri, allocator) {
            CSOUP_ASSERT(Tag::isKnownTag(tagName));
            CSOUP_ASSERT(nodeType == CSOUP_NODE_FORMELEMENT || nodeType == CSOUP_NODE_DOCUMENT);
            
            tag_ = Tag::valueOf(tagName);
            attributes_ = NULL;
            childNodes_ =  NULL;
            classes_ = NULL;
        }
        
        Element(NodeTypeEnum nodeType, const StringRef& tagName, const Attributes& attributes, const StringRef& baseUri, Allocator* allocator) :
        Node(nodeType, NULL, 0, baseUri, allocator) {
            CSOUP_ASSERT(Tag::isKnownTag(tagName));
            CSOUP_ASSERT(nodeType == CSOUP_NODE_FORMELEMENT || nodeType == CSOUP_NODE_DOCUMENT);
            
            tag_ = Tag::valueOf(tagName);
            attributes_ = new (allocator->malloc_t<Attributes>()) Attributes(attributes, allocator);
            childNodes_ =  NULL;
            classes_ = NULL;
        }
        
    private:
        // to be conitnued;
        Node** insert(size_t index) {
            return ensureChildNodes()->insert(index);
        }
        
        Node** append() {
            return ensureChildNodes()->push();
        }
        
        Attributes* ensureAttributes() {
            if (!attributes_) {
                attributes_ = allocator()->malloc_t<Attributes>();
                new (attributes_) Attributes(allocator());
            }
            
            return attributes_;
        }
        
        internal::Vector<Node*>* ensureChildNodes() {
            if (!childNodes_) {
                childNodes_ = allocator()->malloc_t< internal::Vector<Node*> >();
                new (childNodes_) internal::Vector<Node*>(4, allocator());
            }
            
            return childNodes_;
        }
        
        void reindexChildren(size_t from = 0) {
            // we can ignore whether childNodes_ is NULL or not;
            const size_t countOfChildren = childNodeSize();
            
            for (size_t i = from; i < countOfChildren; ++ i) {
                (*childNodes_->at(i))->setSiblingIndex(i);
            }
        }
        
        static void accumulateParents(Element* ele, ElementsRef* output);
        
        static bool isElementNode(Node* node) {
            if (node == NULL) {
                return false;
            }
            
            return node->type() == CSOUP_NODE_ELEMENT ||
                    node->type() == CSOUP_NODE_DOCUMENT ||
                    node->type() == CSOUP_NODE_FORMELEMENT;
        }
        
        friend class Node;
    private:
        Tag* tag_;
        internal::Vector<StringRef>* classes_;
        
        Attributes* attributes_;
        internal::Vector<Node*>* childNodes_;
    };
    
}

#endif