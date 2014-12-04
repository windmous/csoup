#ifndef CSOUP_NODE_H_
#define CSOUP_NODE_H_

namespace csoup {
    template <typename CharType, typename Allocator>
    class GenericNode {
    public:
        GenericNode(Allocator* allocator = NULL) {
            
        }
        
    private:
        
        GenericNode<CharType, Allocator>* parentNode_;
        int siblingIndex;
    };
}

#endif // CSOUP_NODE_H_
