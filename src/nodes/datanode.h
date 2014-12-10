#ifndef CSOUP_DATANODE_H_
#define CSOUP_DATANODE_H_

#include "node.h"
#include "../util/csoup_string.h"

namespace csoup {
    class DataNode : public Node {
    public:
        DataNode(const StringRef& data, Allocator* allocator) :
            Node(CSOUP_NODE_CDATA, NULL, 0, allocator) {
            setWholeData(data);
        }
        
        DataNode(Allocator* allocator) :
            Node(CSOUP_NODE_CDATA, NULL, 0, allocator) {
            text().text_ = NULL;
        }

        
        ~DataNode() {
            if (text().text_) {
                text().text_->~String();
                allocator()->free(text().text_);
                text().text_ = NULL;
            }
        }
        
        void setWholeData(const StringRef& data) {
            if (text().text_) {
                text().text_->~String();
            } else {
                text().text_ = allocator()->malloc_t<String>();
            }
         
            new (text().text_) String(data, allocator());
        }
        
        StringRef wholeData() {
            return text().text_ ? text().text_->ref() : StringRef("");
        }
        
        // Create a new DataNode from HTML encoded data.
    };
    
    CSOUP_STATIC_ASSERT(sizeof(DataNode) == sizeof(Node));
}

#endif

