#ifndef CSOUP_ATTRIBUTE_H_
#define CSOUP_ATTRIBUTE_H_

#include "../util/common.h"
#include "../util/csoup_string.h"

namespace csoup {
    typedef enum {
        CSOUP_ATTR_NAMESPACE_NONE,
        CSOUP_ATTR_NAMESPACE_XLINK,
        CSOUP_ATTR_NAMESPACE_XML,
        CSOUP_ATTR_NAMESPACE_XMLNS,
    } AttributeNamespaceEnum;
    
    class Attribute;
    namespace internal {
        void destroy(Attribute* obj, Allocator* allocator);
    }
    
    class Attribute {
    public:
        Attribute(AttributeNamespaceEnum space, const StringRef& key,
                  const StringRef& value, Allocator* allocator)
        : attrKey_(key, allocator), attrValue_(value, allocator), attrNamespace_(space) {
            CSOUP_ASSERT(attrKey_.data() != NULL);
            CSOUP_ASSERT(attrValue_.data() != NULL);
        }
        
        Attribute(AttributeNamespaceEnum space, const CharType* key,
                  const CharType* value, Allocator* allocator)
        : attrKey_(key, allocator), attrValue_(value, allocator), attrNamespace_(space) {
            CSOUP_ASSERT(key != NULL);
            CSOUP_ASSERT(value != NULL);
        }
        
        const String& key() const {
            return attrKey_;
        }
        
        const String& value() const {
            return attrValue_;
        }
        
        AttributeNamespaceEnum nameSpace() const {
            return attrNamespace_;
        }
        
        Attribute& setKey(const String& key, Allocator* allocator) {
            CSOUP_ASSERT(key.data());
            
            internal::destroy(&attrKey_, allocator);
            attrKey_.~String();
            
            new (&attrKey_) String(key, allocator);
            return *this;
        }
        
        Attribute& setKey(const CharType* key, Allocator* allocator) {
            return setKey(String(key, allocator), allocator);
        }
        
        Attribute& setValue(const String& value, Allocator* allocator) {
            CSOUP_ASSERT(value.data());
            
            internal::destroy(&attrValue_, allocator);
            attrValue_.~String();
            
            new (&attrValue_) String(value, allocator);
            return *this;
        }
        
        Attribute& setValue(const CharType* value, Allocator* allocator) {
            return setValue(String(value, allocator), allocator);
        }
        
//        bool operator == (const Attribute& obj) const {
//            return attrNamespace_ == obj.attrNamespace_ &&
//                    attrKey_.equalsIgnoreCase(obj.key()) &&
//                    attrValue_.equalsIgnoreCase(obj.value());
//        }
        
        friend void internal::destroy(Attribute* obj, Allocator* allocator);
    private:
        Attribute operator = (const Attribute& obj);
        
        String attrKey_;
        String attrValue_;
        AttributeNamespaceEnum attrNamespace_;
    };

    namespace internal {
        inline void destroy(Attribute* obj, Allocator* allocator) {
            destroy(&obj->attrKey_, allocator);
            destroy(&obj->attrValue_, allocator);
        }
    }
}
#endif