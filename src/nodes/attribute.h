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
                         const Ch* name, const Ch* value)
        : attrName_(String(name)), attrValue_(String(value)), attrNamespace_(space) {
            CSOUP_ASSERT(name != NULL);
            CSOUP_ASSERT(value != NULL);
        }
        
        GenericString<Ch> getName() const {
            return attrName_;
        }
        
        GenericString<Ch>& getValue() const {
            return attrValue_;
        }
        
        AttributeNamespaceEnum getNamespace() const {
            return attrNamespace_;
        }
        
        GenericAttribute<Ch>& setKey(const GenericString<Ch>& key) {
            CSOUP_ASSERT(key.data());
            
            attrName_.~GenericString<Ch>();
            new (&attrValue_) GenericString<Ch>(key);
            
            return *this;
        }
        
        GenericAttribute<Ch>& setValue(const GenericString<Ch>& value) {
            CSOUP_ASSERT(value.data());
            
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
        
        template <typename CT, typename Allocator>
        void destroy(GenericAttribute<CT>* obj, Allocator* allocator);
    private:
        GenericString<Ch> attrName_;
        GenericString<Ch> attrValue_;
        AttributeNamespaceEnum attrNamespace_;
    };
    
    typedef GenericAttribute<char> Attribute;
    
    template <typename CharType, typename Allocator>
    inline GenericAttribute<CharType> deepcopy(const GenericAttribute<CharType>& obj,
                                        Allocator* allocator) {
        return GenericAttribute<CharType>(obj.getNamespace(),
                                          deepcopy(obj.getName()),
                                          deepcopy(obj.getValue()));
    }
        
    template <typename CharType, typename Allocator>
    inline void destroy(GenericAttribute<CharType>* obj, Allocator* allocator) {
        destroy(&obj->attrName_);
        destroy(&obj->attrValue_);
    }
}
#endif