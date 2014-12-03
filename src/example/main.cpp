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
#include "../nodes/attribute.h"
#include "../util/csoup_string.h"

using std::cout;
using std::endl;
using namespace csoup;

class Object {
public:
    Object() {
        
    }
    
    ~Object() {
        cout << "destructor is invoked!" << endl;
    }
};

int main(int argc, const char * argv[]) {
    GenericString<char> s("good");
    CrtAllocator allo;
    deepcopy(s, &allo);
    return 0;
}
