#ifndef CSOUP_DOCUMENT_H_
#define CSOUP_DOCUMENT_H_

#include "node.h"
#include "element.h"
#include "../util/allocators.h"

namespace csoup {
    class Document : public Element {
    public:
        Document(Allocator* allocator = NULL) :
                    Element(CSOUP_NAMESPACE_HTML,
                            CSOUP_TAG_HTML,
                            NULL,allocator ? allocator : new MemoryPoolAllocator()),
                    quirksMode_(CSOUP_DOCTYPE_NO_QUIRKS),
                    ownAllocator_(NULL),
                    publicIdentifier_(NULL),
                    systemIdentifier_(NULL),
                    name_(NULL) {
            if (allocator == NULL) {
                ownAllocator_ = Element::allocator();
            }
        }

        ~Document() {
            String::destroyString(&publicIdentifier_, allocator());
            String::destroyString(&systemIdentifier_, allocator());
            String::destroyString(&name_, allocator());

            if (ownAllocator_) {
                delete ownAllocator_;
            }
        }
        
        Element* head() {
            return NULL;
        }
        
        Element* body() {
            return NULL;
        }
        
        StringRef title() {
            return StringRef("");
        }
        
        void setTitle(const StringRef& newTitle) {
            
        }
        
        // createElement
        
        // Moves any text content that is not in the body element into the body.
        // public Document normalise()
        
        // merge multiple <head> or <body> contents into one, delete the remainder, and ensure they are owned by <html>
        // private void normaliseStructure(String tag, Element htmlEl) {
          
        
        // a recursive top-down search stragety to get the node whose tag is tag
        // private Element findFirstElementByTagName(String tag, Node node) {
        
        QuirksModeEnum quirksMode() const {
            return quirksMode_;
        }
        
        void setQuirksMode(QuirksModeEnum quirksMode) {
            quirksMode_ = quirksMode;
        }
    private:
        QuirksModeEnum quirksMode_;
        String* publicIdentifier_;
        String* systemIdentifier_;
        String* name_;
        bool hasDocType_;
        
        Allocator* ownAllocator_;
    };
}

#endif // CSOUP_DOCUMENT_H_