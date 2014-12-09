#ifndef CSOUP_NODE_H_
#define CSOUP_NODE_H_

#include "../utils/common.h"
#include "../internl/vector"
#include "../nodes/tag.h"
#include "../nodes/attributes.h"

namespace csoup {
    namespace internal {
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
            CSOUP_NODE_COMMENT,
            /** Text node, where all contents is whitespace.  v will be a GumboText. */
            CSOUP_NODE_WHITESPACE
        } NodeTypeEnum;
        
        typedef enum {
            CSOUP_DOCTYPE_NO_QUIRKS,
            CSOUP_DOCTYPE_QUIRKS,
            CSOUP_DOCTYPE_LIMITED_QUIRKS
        } QuirksModeEnum;
        
        struct TextNodeData {
            String* text_;
        };
        
        struct ElementData {
            TagNamespaceEnum tagNamespace_;
            TagEnum tag_;
            
            Attributes* attributes_;
            Vector<Node>* childNodes_;
        };
        
        struct DocumentData {
            QuirksModeEnum quirksMode_;
            String* publicIdentifier_;
            String* systemIdentifier_;
            String* name_;
            bool hasDocType_;
            
            Vector<Node>* childNodes_;
            // You may add some options
        };
        
        struct NodeData {
            NodeTypeEnum type_;
            Node* parent_;
            size_t siblingIndex_;
            
            Allocator* allocator_;
            union {
                DocumentData doc_,
                ElementData element_,
                TextNodeData text_;
            } v_;
        };
    }
}

#endif // CSOUP_NODE_H_
