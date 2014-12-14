//
//  token.h
//  csoup
//
//  Created by mac on 12/12/14.
//  Copyright (c) 2014 windpls. All rights reserved.
//

#ifndef CSOUP_TOKEN_H_
#define CSOUP_TOKEN_H_

#include "../util/csoup_string.h"
#include "../util/stringbuffer.h"
#include "../nodes/tag.h"
#include "../nodes/attributes.h"

///////////////////////////////////////////////
// Warning: 1. You must do some attribute namespace stuff!!!!
//          2. You must check if attribute keys confilict!

namespace csoup {
    enum TokenTypeEnum {
        CSOUP_TOKEN_DOCTYPE,
        CSOUP_TOKEN_START_TAG,
        CSOUP_TOKEN_END_TAG,
        CSOUP_TOKEN_COMMENT,
        //CSOUP_TOKEN_WHITESPACE,
        CSOUP_TOKEN_CHARACTER,
        //CSOUP_TOKEN_NULL,
        CSOUP_TOKEN_EOF
    };
    
    class Token {
    public:
        Token(TokenTypeEnum type) : tokenType_(type) {
            
        }
        
        virtual ~Token() = 0;
        
        TokenTypeEnum tokenType() const {
            return tokenType_;
        }
    private:
        TokenTypeEnum tokenType_;
    };
    
    inline Token::~Token() {
        
    }
    
    class DoctypeToken : public Token {
    public:
        DoctypeToken(Allocator* allocator) : Token(CSOUP_TOKEN_DOCTYPE),
                                             name_(NULL),
                                             publicIdentifier_(NULL),
                                             systemIdentifier_(NULL),
                                             forceQuirks_(false) {
            CSOUP_ASSERT(allocator != NULL);
            
            name_ = new (allocator->malloc_t<StringBuffer>()) StringBuffer(allocator);
            publicIdentifier_ = new (allocator->malloc_t<StringBuffer>()) StringBuffer(allocator);
            systemIdentifier_ = new (allocator->malloc_t<StringBuffer>()) StringBuffer(allocator);
        }
        
        ~DoctypeToken() {
            destroy(&name_);
            destroy(&publicIdentifier_);
            destroy(&systemIdentifier_);
        }
        
        StringRef name() const {
            return name_->ref();
        }
        
        void appendName(const StringRef& str) {
            name_->appendString(str);
        }
        
        void appendName(const int c) {
            name_->append(c);
        }
        
        StringRef publicIdentifier() const {
            return publicIdentifier_->ref();
        }
        
        StringRef systemIdentifier() const {
            return systemIdentifier_->ref();
        }
        
        void appendPublicIdentifier(const StringRef& str) {
            publicIdentifier_->appendString(str);
        }
        
        void appendPublicIdentifier(const int c) {
            publicIdentifier_->append(c);
        }
        
        void appendSystemIdentifier(const StringRef& str) {
            name_->appendString(str);
        }

        void appendSystemIdentifier(const int c) {
            name_->append(c);
        }
        
        bool forceQuirks() const {
            return forceQuirks_;
        }
        
        void setForceQuirks(bool flag) {
            forceQuirks_ = flag;
        }
    private:
        StringBuffer* name_;
        StringBuffer* publicIdentifier_;
        StringBuffer* systemIdentifier_;
        bool forceQuirks_;
    };
    
    class TagToken : public Token {
    public:
        TagToken(TokenTypeEnum tokenType, Allocator* allocator)
                                          : Token(tokenType),
                                            tagName_(NULL),
                                            pendingAttributeName_(NULL),
                                            pendingAttributeValue_(NULL),
                                            attributes_(NULL),
                                            selfClosing_(false),
                                            allocator_(allocator) {
            
        }
        
        virtual ~TagToken() = 0;
        
        void setTagName(const StringRef& name) {
            if(!tagName_) {
                tagName_ = new (allocator_->malloc_t<StringBuffer>()) StringBuffer(allocator_);
            }
            
            tagName_->clear();
            tagName_->appendString(name);
        }
        
        void finaliseTag() {
            if (pendingAttributeName_ != NULL && pendingAttributeName_->size() > 0) {
                newAttribute();
            }
        }
        
        StringRef tagName() const {
            CSOUP_ASSERT(tagName_ != NULL);
            return tagName_->ref();
        }
        
        TagEnum tag() const {
            CSOUP_ASSERT(tagName_ != NULL);
            return tagEnumFromText(tagName_->data());
        }
        
        bool selfClosing() const {
            return selfClosing_;
        }
        
        void setSelfClosing() {
            selfClosing_ = true;
        }
        
        Attributes* attributes() {
            return attributes_;
        }
        
    //protected:
        void newAttribute() {
            ensureAttributes();
            
            if (pendingAttributeName_ != NULL) {
                CSOUP_ASSERT(pendingAttributeName_->size() > 0);
                
                if (pendingAttributeValue_ != NULL) {
                    attributes_->addAttribute(CSOUP_ATTR_NAMESPACE_NONE,
                                              pendingAttributeName_->ref(),
                                              pendingAttributeValue_->ref());
                    pendingAttributeValue_->clear();
                } else {
                    attributes_->addAttribute(CSOUP_ATTR_NAMESPACE_NONE,
                                              pendingAttributeName_->ref(),
                                              StringRef(""));
                }
                
                pendingAttributeName_->clear();
            }
        }
        
        void appendTagName(int codePoint) {
            ensureStringBuffer(&tagName_);
            tagName_->append(codePoint);
        }
        
        void appendTagName(const StringRef& str) {
            ensureStringBuffer(&tagName_);
            tagName_->appendString(str);
        }
        
        void appendAttributeName(int codePoint) {
            ensureStringBuffer(&pendingAttributeName_);
            pendingAttributeName_->append(codePoint);
        }
        
        void appendAttributeName(const StringRef& str) {
            ensureStringBuffer(&pendingAttributeName_);
            pendingAttributeName_->appendString(str);
        }
        
        void appendAttributeValue(int codePoint) {
            ensureStringBuffer(&pendingAttributeValue_);
            pendingAttributeValue_->append(codePoint);
        }
        
        void appendAttributeValue(const StringRef& str) {
            ensureStringBuffer(&pendingAttributeValue_);
            pendingAttributeValue_->appendString(str);
        }
        
        inline void ensureStringBuffer(StringBuffer** buffer) {
            if (*buffer == NULL) {
                *buffer = new (allocator_->malloc_t<StringBuffer>()) StringBuffer(allocator_);
            }
        }
        
        inline void ensureAttributes() {
            if (attributes_ == NULL) {
                attributes_ = new (allocator_->malloc_t<Attributes>()) Attributes(allocator_);
            }
        }
    
    private:
        StringBuffer* tagName_;
        StringBuffer* pendingAttributeName_;
        StringBuffer* pendingAttributeValue_;
        Attributes* attributes_;
        bool selfClosing_;
        
        Allocator* allocator_;
    };
    
    inline TagToken::~TagToken() {
        destroy(&tagName_);
        destroy(&pendingAttributeName_);
        destroy(&pendingAttributeValue_);
        destroy(&attributes_, attributes_->allocator());
    }
    
    class StartTagToken : public TagToken {
    public:
        StartTagToken(Allocator* allocator) : TagToken(CSOUP_TOKEN_START_TAG, allocator) {
            CSOUP_ASSERT(allocator != NULL);
        }
        
        StartTagToken(const StringRef& name, Allocator* allocator) : TagToken(CSOUP_TOKEN_START_TAG, allocator) {
            CSOUP_ASSERT(allocator != NULL);
            setTagName(name);
        }
    };
    
    class EndTagToken : public TagToken {
    public:
        EndTagToken(Allocator* allocator) : TagToken(CSOUP_TOKEN_END_TAG, allocator) {
            CSOUP_ASSERT(allocator != NULL);
        }
        
        EndTagToken(const StringRef& name, Allocator* allocator) : TagToken(CSOUP_TOKEN_START_TAG, allocator) {
            CSOUP_ASSERT(allocator != NULL);
            setTagName(name);
        }
    };
    
    class CommentToken : public Token {
    public:
        CommentToken(Allocator* allocator) : Token(CSOUP_TOKEN_COMMENT),
                                             bogus_(false),
                                             content_(NULL) {
            CSOUP_ASSERT(allocator != NULL);
            content_ = new (allocator->malloc_t<StringBuffer>()) StringBuffer(allocator);
        }
        
        ~CommentToken() {
            destroy(&content_);
        }
        
        void append(int c) {
            content_->append(c);
        }
        
        void append(const StringRef& str) {
            content_->appendString(str);
        }
        
        StringRef data() const {
            return content_->ref();
        }
        
        bool bogus() const {
            return bogus_;
        }
        
        void setBogus(bool flag) {
            bogus_ = flag;
        }
    private:
        bool bogus_;
        StringBuffer* content_;
    };
    
    class CharacterToken : public Token {
    public:
        CharacterToken(const StringRef& str, Allocator* allocator) : Token(CSOUP_TOKEN_CHARACTER) {
            CSOUP_ASSERT(allocator != NULL);
            data_ = new (allocator->malloc_t<String>()) String(str, allocator);
        }
        
        ~CharacterToken() {
            destroy(&data_);
        }
        
        StringRef data() const {
            return data_->ref();
        }
        
    private:
        String* data_;
    };
    
    class EOFToken : public Token {
    public:
        EOFToken() : Token(CSOUP_TOKEN_EOF) {
            
        }
    };
}

#endif // CSOUP_TOKEN_H_
