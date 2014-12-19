#ifndef CSOUP_DOCUMENT_H_
#define CSOUP_DOCUMENT_H_

#include "element.h"

namespace csoup {
    class Document : public Element {
    public:
        Document(const StringRef& baseUri, Allocator* allocator = NULL);
        Document(const StringRef& baseUri, const Attributes& attributes, Allocator* allocator = NULL);
        ~Document();
        
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
        
        StringRef baseUri() const {
            return baseUri_->ref();
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
        
        void setPublicIdentifier(const StringRef& publicIdentifier);
        
        StringRef publicIdentifier() const {
            CSOUP_ASSERT(publicIdentifier_ != NULL);
            return publicIdentifier_->ref();
        }
        
        void setSystemIdentifier(const StringRef& systemIdentifier);
        
        StringRef systemIdentifier() const {
            CSOUP_ASSERT(systemIdentifier_ != NULL);
            return systemIdentifier_->ref();
        }
        
        void setName(const StringRef& name);
        
        StringRef name() const {
            return name_->ref();
        }
        
    private:
        QuirksModeEnum quirksMode_;
        String* publicIdentifier_;
        String* systemIdentifier_;
        String* name_;
        String* baseUri_;
        bool hasDocType_;
        
        Allocator* ownAllocator_;
    };
}

#endif // CSOUP_DOCUMENT_H_