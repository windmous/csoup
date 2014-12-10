#ifndef CSOUP_COMMENT_H_
#define CSOUP_COMMENT_H_

#include "node.h"
#include "../util/csoup_string.h"

namespace csoup {
    class Comment : public Node {
    public:
        Comment(const StringRef& comment, Allocator* allocator) :
            Node(CSOUP_NODE_COMMENT, NULL, 0, allocator) {
            setComment(comment);
        }
        
        Comment(Allocator* allocator) : Node(CSOUP_NODE_COMMENT, NULL, 0, allocator) {
            text().text_ = NULL;
        }
        
        ~Comment() {
            if (text().text_) {
                text().text_->~String();
                allocator()->free(text().text_);
                text().text_ = NULL;
            }
        }
        
        void setComment(const StringRef& data) {
            if (text().text_) {
                text().text_->~String();
            } else {
                text().text_ = allocator()->malloc_t<String>();
            }
            
            new (text().text_) String(data, allocator());
        }
        
        StringRef comment() {
            return text().text_ ? text().text_->ref() : StringRef("");
        }
        
        // Create a new DataNode from HTML encoded data.
    };
    
    CSOUP_STATIC_ASSERT(sizeof(Comment) == sizeof(Node));
}

#endif

