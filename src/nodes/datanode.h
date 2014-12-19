#ifndef CSOUP_DATANODE_H_
#define CSOUP_DATANODE_H_

#include "node.h"
#include "../util/csoup_string.h"

namespace csoup {
    class DataNode : public Node {
    public:
        DataNode(const StringRef& data, const StringRef& baseUri, Allocator* allocator) :
            Node(CSOUP_NODE_CDATA, NULL, 0, baseUri, allocator) {
            setWholeData(data);
        }
        
        DataNode(const StringRef& baseUri, Allocator* allocator) :
            Node(CSOUP_NODE_CDATA, NULL, 0, baseUri, allocator) {
            data_ = NULL;
        }

        
        ~DataNode() {
            if (data_) {
                data_->~String();
                allocator()->free(data_);
                data_ = NULL;
            }
        }
        
        void setWholeData(const StringRef& data) {
            if (data_) {
                data_->~String();
            } else {
                data_ = allocator()->malloc_t<String>();
            }
         
            new (data_) String(data, allocator());
        }
        
        StringRef wholeData() {
            return data_ ? data_->ref() : StringRef("");
        }
        
    private:
        String* data_;
        // Create a new DataNode from HTML encoded data.
    };
}

#endif

