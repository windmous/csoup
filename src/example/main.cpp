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

int main(int argc, const char * argv[]) {
    using csoup::internal::Stack;
    
    // insert code here...
    std::cout << "Hello, World!\n" << std::endl;
    Stack<csoup::MemoryPoolAllocator<csoup::CrtAllocator> > pStack(NULL, 10);
    
    return 0;
}
