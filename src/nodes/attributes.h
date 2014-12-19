#ifndef CSOUP_ATTRIBUTES_H_
#define CSOUP_ATTRIBUTES_H_

#include "../internal/vector.h"
#include "../util/allocators.h"
#include "../util/csoup_string.h"
#include "attribute.h"

namespace csoup {
    class Attributes {
    public:
        Attributes(Allocator* allocator) : allocator_(allocator),
                                            attributes_(NULL) {
            CSOUP_ASSERT(allocator != NULL);
        }
        
        ~Attributes() {
            if (!allocator_ || !attributes_) return;
            
            attributes_->~Vector();
            allocator_->free(attributes_);
        }
        
        Attributes(const Attributes& attrs, Allocator* allocator) : allocator_(allocator) {
            CSOUP_ASSERT(allocator != NULL);
            if (attrs.size() == 0) {
                return ;
            }
            
            // Use a very naive style;  Vector didn't have a copy constructor
            for (size_t i = 0; i < attrs.size(); ++ i) {
                const Attribute* attr = attrs.get(i);
                addAttribute(attr->key(), attr->value());
            }
        }
        
        StringRef get(AttributeNamespaceEnum space, const StringRef& key) const {
            if (!attributes_ || !key.size())
                return StringRef("");
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (isAttributeHasKey(attributes_->at(i), space, key))
                    return attributes_->at(i)->value().ref();
            }
            
            return StringRef("");
        }
        
        StringRef get(const StringRef& key) const {
            return get(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        const Attribute* get(const size_t index) const {
            CSOUP_ASSERT(index < size());
            return attributes_->at(index);
        }
        
        void removeAttribute(AttributeNamespaceEnum space, const StringRef& key) {
            if (!attributes_ || !key.size()) return ;
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (isAttributeHasKey(attributes_->at(i), space, key)) {
                    attributes_->remove(i);
                    return ;
                }
            }
        }
        
        void removeAttribute(const StringRef& key) {
            return removeAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        void addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value) {
            if (!key.size()) return ;
            if (!attributes_) {
                attributes_ = allocator_->malloc_t< internal::Vector<Attribute> >();
            }
            
            // try to remove the attribute entry 
            removeAttribute(space, key);
            
            Attribute* newAttribute = attributes_->push();
            new (newAttribute) Attribute(space, key, value, allocator_);
        }
        
        void addAttribute(const StringRef& key,const StringRef& value) {
            addAttribute(CSOUP_ATTR_NAMESPACE_NONE, key, value);
        }

        
        bool hasAttribute(AttributeNamespaceEnum space, const StringRef& key) const {
            if (!key.size() || !attributes_) return false;
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (isAttributeHasKey(attributes_->at(i), space, key))
                    return true;
            }
            
            return false;
        }
        
        bool hasAttribute(const StringRef& key) const {
            return hasAttribute(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        size_t size() const {
            return attributes_ == NULL ? 0 : attributes_->size();
        }
        
        Allocator* allocator() {
            return allocator_;
        }
        
        bool equals(const Attributes& obj) const {
            return true;
        }
        //friend void internal::destroy(Attributes* attributes, Allocator* allocator);
    private:
        Attributes(const Attributes&);
        Attributes operator=(const Attributes&);
        
        static bool isAttributeHasKey(const Attribute* attr,
                            AttributeNamespaceEnum space, const StringRef& key) {
            return attr->nameSpace() == space && \
                internal::strEqualsIgnoreCase(attr->key(), key);
        }
        
        Allocator* allocator_;
        internal::Vector<Attribute>* attributes_;
    };
}

#endif // CSOUP_ATTRIBUTES_H_
