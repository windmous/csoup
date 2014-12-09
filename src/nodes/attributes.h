#ifndef CSOUP_ATTRIBUTES_H_
#define CSOUP_ATTRIBUTES_H_

#include "../internal/vector.h"
#include "../util/allocators.h"
#include "../util/csoup_string.h"
#include "attribute.h"

namespace csoup {
    namespace internal {
        class Attributes;
        void destroy(Attributes* attributes, Allocator* allocator);
    }
    
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
        
        void remove(AttributeNamespaceEnum space, const StringRef& key) {
            if (!attributes_ || !key.size()) return ;
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (isAttributeHasKey(attributes_->at(i), space, key)) {
                    attributes_->remove(i);
                    return ;
                }
            }
        }
        
        void remove(const StringRef& key) {
            return remove(CSOUP_ATTR_NAMESPACE_NONE, key);
        }
        
        bool addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value) {
            if (!key.size() || hasAttribute(space, key)) return false;
            if (!attributes_) {
                attributes_ = allocator_->malloc_t< internal::Vector<Attribute> >();
            }
            
            Attribute* newAttribute = attributes_->push();
            new (newAttribute) Attribute(space, key, value, allocator_);
            
            return true;
        }
        
        bool addAttribute(const StringRef& key,const StringRef& value) {
            return addAttribute(CSOUP_ATTR_NAMESPACE_NONE, key, value);
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
        
        //friend void internal::destroy(Attributes* attributes, Allocator* allocator);
    private:
        static bool isAttributeHasKey(const Attribute* attr,
                            AttributeNamespaceEnum space, const StringRef& key) {
            return attr->nameSpace() == space && \
                internal::strEqualsIgnoreCase(attr->key(), key);
        }
        
        Allocator* allocator_;
        internal::Vector<Attribute>* attributes_;
    };
    
    
//    namespace internal {
//        void destroy(Attributes* attributes, Allocator* allocator) {
//            attributes->~Attributes();
//        }
//    }

}

#endif // CSOUP_ATTRIBUTES_H_
