//
//  elements.h
//  csoup
//
//  Created by mac on 12/19/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_ELEMENTSREF_H_
#define CSOUP_ELEMENTSREF_H_

#include "../internal/vector.h"
#include "../util/stringref.h"
#include "../util/stringbuffer.h"

namespace csoup {
    class Element;
    
    class ElementsRef {
    public:
        ElementsRef(Allocator* allocator) : contents_(1, allocator) {
            
        }
        
        ElementsRef(size_t initialCapacity, Allocator* allocator) : contents_(initialCapacity, allocator) {
            
        }
        
        ElementsRef(const ElementsRef& obj, Allocator* allocator) : contents_(obj.size(), allocator) {
            add(obj);
        }
        
        size_t size() const {
            return contents_.size();
        }
        
        void add(const ElementsRef& obj) {
            contents_.reserve(contents_.size() + obj.size());
            for (size_t i = 0; i < obj.size(); ++ i)
                contents_.push(*obj.contents_.at(i));
        }
        
        StringRef attr(const StringRef& key) const;
        
        bool hasAttribute(const StringRef& key) const;
        
        void addAttribute(const StringRef& key, const StringRef& value);
        
        void removeAttribute(const StringRef& key);
        
        void addClass(const StringRef& className);
        
        void removeCLass(const StringRef& className);
        
        void toggleClass(const StringRef& className);
        
        bool hasClass(const StringRef& className);
        
        StringRef val() const;
        
        void setVal(const StringRef& value);
        
        void text(StringBuffer* buffer);
        
        bool hasText() const;
        
        void setTagName(const StringRef& tagName);
        
        
        // we may don't support fragment
        void prepend(const StringRef& html);
        
        void append(const StringRef& html);
        
        void before(const StringRef& html);
        
        void after(const StringRef& html);
        
        void wrap(const StringRef& html);
        
        void unwrap();
        
        void setElementsEmpty();
        
        void removeFromParent();
        
        void select(const StringRef& query, ElementsRef* output);
        
        void notQuery(const StringRef& query, ElementsRef* output);
        
        void eq(size_t index, ElementsRef* output);
        
        bool is(const StringRef& query);
        
        void parents(ElementsRef* output);
        
        Element* first() {
            return contents_.empty() ? NULL : *contents_.at(0);
        }
        
        Element* last() {
            return contents_.empty() ? NULL : *contents_.back();
        }
        
        bool empty() const {
            return contents_.empty();
        }
        
        void append(Element* ele) {
            contents_.push(ele);
        }
        
        size_t indexOf(const Element* ele) {
            for (size_t i = 0; i < contents_.size(); ++ i) {
                if (*contents_.at(i) == ele) {
                    return i;
                }
            }
            
            return contents_.size();
        }
        
        size_t lastIndexOf(const Element* ele) {
            for (size_t i = contents_.size(); i > 0; -- i) {
                if (*contents_.at(i - 1) == ele) {
                    return i - 1;
                }
            }
            
            return contents_.size();
        }
        
        void insert(const size_t index, Element* el) {
            contents_.insert(index, el);
        }
        
        Element* get(size_t index) {
            return *contents_.at(index);
        }
        
        
    private:
        ElementsRef(const ElementsRef&);
        
        internal::Vector<Element*> contents_;
    };
}

#endif // CSOUP_ELEMENTSREF_H_
