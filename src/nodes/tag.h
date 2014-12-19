#ifndef CSOUP_TAG_H_
#define CSOUP_TAG_H_

#include "../util/common.h"
#include "../util/stringref.h"

namespace csoup {
    class StringRef;
    
    class Tag {
    public:
        StringRef tagName() const {
            return tagName_;
        }
        
        static Tag* valueOf(const StringRef& tagName) {
            static Tag::GlobalTagMap globalTagMap;
            
            return globalTagMap.query(tagName);
        }
        
        bool block() const {
            return isBlock_;
        }
        
        bool formatAsBlock() const {
            return formatAsBlock_;
        }
        
        bool canContainBlock() const {
            return canContainBlock_;
        }
        
        bool inlineTag() const {
            return !isBlock_;
        }
        
        bool data() const {
            return !canContainInline_ && !empty();
        }
        
        bool empty() const {
            return empty_;
        }
        
        bool selfClosing() const {
            return empty_ || selfClosing_;
        }
        
        bool isKnownTag() const {
            return valueOf(tagName()) != NULL;
        }
        
        static bool isKnownTag(const StringRef& tagName) {
            return valueOf(tagName) != NULL;
        }
        
        bool preserveWhitespace() const {
            return preserveWhitespace_;
        }
        
        bool formListed() const {
            return formList_;
        }
        
        bool formSubmittable() const {
            return formSubmit_;
        }
        
        void setSelfClosing() {
            selfClosing_ = true;
        }
        
        bool operator == (const Tag& obj) const;
        
    private:
        Tag(const StringRef& tagName);
        
        struct GlobalTagMap {
            GlobalTagMap();
            ~GlobalTagMap();
            
            Tag* query(const StringRef& tagName);
        private:
            void registerTag(const StringRef& tagName, Tag* tag);
        };
        
        StringRef tagName_;
        
        // Use a bit to optimize this;
        bool isBlock_; // block or inline
        bool formatAsBlock_; // should be formatted as a block
        bool canContainBlock_; // Can this tag hold block level tags?
        bool canContainInline_; // only pcdata if not
        bool empty_ ; // can hold nothing; e.g. img
        bool selfClosing_; // can self close (<foo />). used for unknown tags that self close, without forcing them as empty.
        bool preserveWhitespace_; // for pre, textarea, script etc
        bool formList_; // a control that appears in forms: input, textarea, output etc
        bool formSubmit_; // a control that can be submitted in a form: input etc
    };
}

#endif