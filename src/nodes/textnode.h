#ifndef CSOUP_TEXTNODE_H_
#define CSOUP_TEXTNODE_H_

#include "node.h"
#include "../util/csoup_string.h"

namespace csoup {
    class TextNode : public Node {
    public:
        TextNode(const StringRef& text, const StringRef& baseUri, Allocator* allocator) :
            Node(CSOUP_NODE_TEXT, NULL, 0, baseUri, allocator) {
            setWholeText(text);
        }
        
        ~TextNode() {
            if (text_) {
                text_->~String();
                allocator()->free(text_);
                text_ = NULL;
            }
        }
        
        void setWholeText(const StringRef& data) {
            if (text_) {
                text_->~String();
            } else {
                text_ = allocator()->malloc_t<String>();
            }
            
            new (text_) String(data, allocator());
        }
        
        // you should return normaliseWhitespace text
        // Normalise the whitespace within this string; multiple spaces collapse to a single, and all whitespace characters
        StringRef wholeText() {
            return text_ ? text_->ref() : StringRef("");
        }
        
        // Create a new DataNode from HTML encoded data.
        
        // TODO:
        // stripLeadingWhitespace
        // lastCharIsWhitespace
        // createFromEncoded
        // normaliseWhitespace
        // isBlank Test if this text node is blank -- that is, empty or only whitespace (including newlines).
        // splitText
    private:
        String* text_;
    };
    
    //CSOUP_STATIC_ASSERT(sizeof(TextNode) == sizeof(Node));
}

#endif // CSOUP_TEXTNODE_H_

