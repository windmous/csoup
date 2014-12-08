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
        
        StringRef get(const StringRef& key) {
            if (!attributes_ || !key.size())
                return StringRef("");
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (internal::strEqualsIgnoreCase(attributes_->at(i)->key(),key))
                    return attributes_->at(i)->value().ref();
            }
            
            return StringRef("");
        }
        
        void remove(const StringRef& key) {
            if (!attributes_) return ;
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                internal::destroy(attributes_->at(i), allocator_);
                attributes_->remove(i);
                break;
            }
        }
        
        bool addAttribute(AttributeNamespaceEnum space, const StringRef& key,
                          const StringRef& value, Allocator* allocator) {
            if (hasAttribute(key)) return false;
            if (!attributes_) {
                attributes_ = allocator_->malloc_t< internal::Vector<Attribute> >();
            }
            
            Attribute* newAttribute = attributes_->push();
            new (newAttribute) Attribute(space, key, value, allocator);
            
            return true;
        }
        
        bool hasAttribute(const StringRef& key) {
            if (!attributes_) return false;
            
            for (size_t i = 0; i < attributes_->size(); ++ i) {
                if (internal::strEqualsIgnoreCase(attributes_->at(i)->key(), key))
                    return true;
            }
            
            return false;
        }
        
        size_t size() const {
            return attributes_ == NULL ? 0 : attributes_->size();
        }
        
        
        
    private:
        Allocator* allocator_;
        internal::Vector<Attribute>* attributes_;
    };
}

#endif // CSOUP_ATTRIBUTES_H_
