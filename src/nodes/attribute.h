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
    
    template <typename CharType = char>
    class GenericAttribute {
    public:
        typedef CharType Ch;
        
        GenericAttribute(AttributeNamespaceEnum space,
                         const GenericString<Ch>& name,
                         const GenericString<Ch>& value)
        : attrName_(name), attrValue_(value), attrNamespace_(space) {
            CSOUP_ASSERT(attrName_.data() != NULL);
            CSOUP_ASSERT(attrValue_.data() != NULL);
        }
        
        GenericAttribute(AttributeNamespaceEnum space,
                         const Ch* name, const Ch* value) {
            
        }
        
        const GenericString<Ch> getName() const {
            return attrName_;
        }
        
        const GenericString<Ch>& getValue() const {
            return attrValue_;
        }
        
        AttributeNamespaceEnum getNamespace() const {
            return attrNamespace_;
        }
        
        GenericAttribute<Ch>& setKey(const GenericString<Ch>& key) {
            CSOUP_ASSERT(key.s);
            
            attrName_.~GenericString<Ch>();
            new (&attrValue_) GenericString<Ch>(key);
            
            return *this;
        }
        
        GenericAttribute<Ch>& setValue(const GenericString<Ch>& value) {
            CSOUP_ASSERT(value.s);
            
            attrValue_.~GenericString<Ch>();
            new (&attrValue_) GenericString<Ch>(value);
            
            return *this;
        }
        
        template <typename Allocator>
        GenericAttribute<Ch> clone(Allocator* allocator) {
            return GenericAttribute(attrNamespace_,
                                    attrName_.clone(allocator),
                                    attrValue_.clone(allocator));
        }
        
        template <typename Allocator>
        GenericAttribute<Ch> destroy(Allocator* allocator) {
            attrName_.template destroy<Allocator>(allocator);
            attrValue_.template destroy<Allocator>(allocator);
        }
        
    private:
        GenericString<Ch> attrName_;
        GenericString<Ch> attrValue_;
        AttributeNamespaceEnum attrNamespace_;
    };
}
#endif