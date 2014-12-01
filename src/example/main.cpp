//
//  main.cpp
//  CSoup
//
//  Created by mac on 11/28/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#include <iostream>
#include "../internal/stack.h"
#include "../util/allocators.h"
#include "../internal/vector.h"
#include "../internal/bytearray.h"

using std::cout;
using std::endl;

class Object {
public:
    Object() {
        
    }
    
    ~Object() {
        cout << "destructor is invoked!" << endl;
    }
};

int main(int argc, const char * argv[]) {
    //using csoup::internal::Vector;
    using csoup::internal::Vector;
    using csoup::internal::Stack;
    Stack<csoup::CrtAllocator> s(NULL,10);
    s.template push<Object>(Object());
    s.template popAndDestroy<Object>();

    // insert code here...
    //Vector<int, csoup::CrtAllocator> vec(NULL, 1);
    Vector<int, csoup::CrtAllocator> ba(NULL, 10);
    for (int i = 0; i < 10; ++ i) {
        ba.push(i);
    }
    for (int i = (int)ba.getSize() - 1; i >= 0; -- i) {
        cout << *ba.get(i) << " ";
    }
    cout << endl;
    
    while (!ba.empty()) {
        cout << *ba.pop() << endl;
    }
    
    Vector<Object, csoup::MemoryPoolAllocator<>> vec(NULL, 10);
    *vec.push() = Object();
    
    
    return 0;
}
