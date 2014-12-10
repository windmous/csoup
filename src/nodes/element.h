#ifndef CSOUP_ELEMENT_H_
#define CSOUP_ELEMENT_H_

#include "../internal/nodedata.h"
#include "attributes.h"
#include "node.h"
#include "tag.h"

namespace csoup {
    class Element : public Node {
    public:
        Element(TagNamespaceEnum space, TagEnum tag, Attributes* attributes, Allocator* allocator) :
                Node(CSOUP_NODE_ELEMENT, NULL, 0, allocator) {
            element().tagNamespace_ = space;
            element().tag_ = tag;
            element().attributes_ = attributes;
            element().childNodes_ =  NULL;
        }
        
        ~Element() {
            Attributes* attributes = element().attributes_;
            if (attributes) {
                attributes->~Attributes();
                allocator()->free(attributes);
            }
            
            internal::Vector<Node>* childNodes = element().childNodes_;
            if (childNodes) {
                childNodes->~Vector<Node>();
                allocator()->free(childNodes);
            }
        }
        
        //////////////////////////////////////////////////
        // Methods about element
        
        TagEnum tag() const {
            return element().tag_;
        }
        
        StringRef tagName() const {
            return tagEnumToName(tag());
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
        
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        void before() {}
        void after() {}
        //////////////////////////////////////////////////
        // Methods about attributes
        
        StringRef attr(const StringRef& key) const {
            return attr(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        StringRef attr(AttributeNamespaceEnum space, const StringRef& key) const {
            return element().attributes_ ? element().attributes_->get(key) : StringRef("");
        }
        
        const Attributes* attributes() const {
            return element().attributes_;
        }
        
        bool addAttribute(const StringRef& key, const StringRef& value) {
            return addAttribute(CSOUP_ATTR_NAMESPACE_NONE, key, value);
        }
        
        bool addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value) {
            return ensureAttributes()->addAttribute(space, key, value);
        }
        
        bool hasAttribute(const StringRef& key) const {
            return hasAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        bool hasAttribute(AttributeNamespaceEnum space, const StringRef& key) const {
            return element().attributes_ ? element().attributes_->hasAttribute(space, key) : false;
        }
        
        void removeAttribute(const StringRef& key) {
            return removeAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        void removeAttribute(AttributeNamespaceEnum space, const StringRef& key) {
            if (element().attributes_) element().attributes_->removeAttribute(space, key);
        }
        
        ////////////////////////////////////////////////
        // Methods about children node
        size_t childNodeSize() const {
            return element().childNodes_ ? element().childNodes_->size() : 0;
        }
        
        Node* childNode(size_t index) const {
            CSOUP_ASSERT(index < childNodeSize());
            return element().childNodes_->at(index);
        }
        
        void removeChild(size_t index) {
            CSOUP_ASSERT(index < childNodeSize());
            
            // the node in vector would be destroyed
            element().childNodes_->remove(index);
            reindexChildren();
        }
        
        void removeChild(Node** node) {
            CSOUP_ASSERT((*node)->parentNode() == this);
            removeChild((*node)->siblingIndex());
            *node = NULL;
        }
        
        // to be conitnued;
        Node* addChild(size_t index, NodeTypeEnum type) {
            Node* ret = ensureChildNodes()->insert(index);
            reindexChildren();
            return ret;
        }
        
        ///////////////////////////////////////////////
        // !!!!!!!!!!!!!!!!
        Node* addChild(NodeTypeEnum type) {
            Node* ret = ensureChildNodes()->push();
            
            // we don't need re-ind;ex operation
            reindexChildren(childNodeSize() - 1);
            return ret;
        }
        
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
                arrayBuffer[i] = element().childNodes_->at(i);
            }
            
            return countOfChildren;
        }
        
        //void addChild(Node* node)
        
    private:
        Attributes* ensureAttributes() {
            if (!element().attributes_) {
                element().attributes_ = allocator()->malloc_t<Attributes>();
                new (element().attributes_) Attributes(allocator());
            }
            
            return element().attributes_;
        }
        
        internal::Vector<Node>* ensureChildNodes() {
            if (!element().childNodes_) {
                element().childNodes_ = allocator()->malloc_t< internal::Vector<Node> >();
                new (element().childNodes_) internal::Vector<Node>(allocator(), 4);
            }
            
            return element().childNodes_;
        }
        
        void reindexChildren(size_t from = 0) {
            // we can ignore whether childNodes_ is NULL or not;
            const size_t countOfChildren = childNodeSize();
            
            for (size_t i = from; i < countOfChildren; ++ i) {
                element().childNodes_->at(i)->setSiblingIndex(i);
            }
        }
    };
    
    CSOUP_STATIC_ASSERT(sizeof(Element) == sizeof(Node));
}

#endif