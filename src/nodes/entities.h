#ifndef CSOUP_ENTITIES_H_
#define CSOUP_ENTITIES_H_

#include "../util/common.h"

namespace csoup {
    class StringRef;
    class Allocator;
    
    class Entities {
    public:
        static bool isNamedEntity(const StringRef& name, Allocator* allocator);
        
        static bool isNamedEntity(const CharType* name);
        
        static bool isBaseNamedEntity(const StringRef& name, Allocator* allocator);
        
        static bool isBaseNamedEntity(const CharType* name);
        
        static int getCharacterByName(const StringRef& name, Allocator* allocator);
        
        static int getCharacterByName(const CharType* name);
        
        
    };
}

#endif // CSOUP_ENTITIES_H_