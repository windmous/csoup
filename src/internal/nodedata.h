#ifndef CSOUP_INTERNAL_NODEDATA_H_
#define CSOUP_INTERNAL_NODEDATA_H_

#include "../util/common.h"
#include "../nodes/tag.h"
#include "vector.h"

namespace csoup {
    class Attributes;
    class String;
    class Node;
    
    typedef enum {
        /** Document node.  v will be a GumboDocument. */
        CSOUP_NODE_DOCUMENT,
        /** Element node.  v will be a GumboElement. */
        CSOUP_NODE_ELEMENT,
        /** Text node.  v will be a GumboText. */
        CSOUP_NODE_TEXT,
        /** CDATA node. v will be a GumboText. */
        CSOUP_NODE_CDATA,
        /** Comment node.  v. will be a GumboText, excluding comment delimiters. */
        CSOUP_NODE_COMMENT//,
        /** Text node, where all contents is whitespace.  v will be a GumboText. */
        //CSOUP_NODE_WHITESPACE
    } NodeTypeEnum;
    
    typedef enum {
        CSOUP_DOCTYPE_NO_QUIRKS,
        CSOUP_DOCTYPE_QUIRKS,
        CSOUP_DOCTYPE_LIMITED_QUIRKS
    } QuirksModeEnum;
    
    namespace internal {
        struct TextNodeData {
            String* text_;
        };
        
        template <typename T>
        class Vector;

        
        struct ElementData {
            TagNamespaceEnum tagNamespace_;
            TagEnum tag_;
            
            Attributes* attributes_;
            Vector<Node>* childNodes_;
        };
        
//        struct DocumentData {
//            QuirksModeEnum quirksMode_;
//            String* publicIdentifier_;
//            String* systemIdentifier_;
//            String* name_;
//            bool hasDocType_;
//            
//            Vector<Node>* childNodes_;
//            // You may add some options
//        };
        
        struct NodeData {
            NodeData(NodeTypeEnum type, Node* parent, size_t index, Allocator* allocator) :
                    type_(type), parent_(parent),
                    siblingIndex_(index), allocator_(allocator) {
                CSOUP_ASSERT(allocator_ != NULL);
            }
            
            NodeTypeEnum type_;
            Node* parent_;
            size_t siblingIndex_;
            
            Allocator* allocator_;
            union {
                //DocumentData doc_;
                ElementData element_;
                TextNodeData text_;
            } v_;
            
        private:
            NodeData(const NodeData&);
            NodeData operator = (const NodeData* );
        };
    }
}

#endif // CSOUP_NODE_H_
