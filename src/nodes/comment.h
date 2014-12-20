#ifndef CSOUP_COMMENT_H_
#define CSOUP_COMMENT_H_

#include "node.h"
#include "../util/csoup_string.h"

namespace csoup {
    class CommentNode : public Node {
    public:
        CommentNode(const StringRef& comment, const StringRef& baseUri, Allocator* allocator) :
            Node(CSOUP_NODE_COMMENT, NULL, 0, baseUri, allocator) {
            setComment(comment);
        }
        
        CommentNode(const StringRef& baseUri, Allocator* allocator) : Node(CSOUP_NODE_COMMENT, NULL, 0, baseUri, allocator) {
            comment_ = NULL;
        }
        
        ~CommentNode() {
            if (comment_) {
                comment_->~String();
                allocator()->free(comment_);
                comment_ = NULL;
            }
        }
        
        void setComment(const StringRef& data) {
            if (comment_) {
                comment_->~String();
            } else {
                comment_ = allocator()->malloc_t<String>();
            }
            
            new (comment_) String(data, allocator());
        }
        
        StringRef comment() {
            return comment_ ? comment_->ref() : StringRef("");
        }
        
        // Create a new DataNode from HTML encoded data.
    private:
        String* comment_;
    };
    

}

#endif

