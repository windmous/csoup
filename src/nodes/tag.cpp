#include <map>
#include <string>
#include "tag.h"
#include "../util/stringref.h"

namespace {
    using namespace csoup;
    
    typedef std::map<std::string, Tag*> TagMap;
    TagMap globalTags_;
    
    // internal static initialisers:
    // prepped from http://www.w3.org/TR/REC-html40/sgml/dtd.html and other sources
    StringRef blockTags[] = {
        "html", "head", "body", "frameset", "script", "noscript", "style", "meta", "link", "title", "frame",
        "noframes", "section", "nav", "aside", "hgroup", "header", "footer", "p", "h1", "h2", "h3", "h4", "h5", "h6",
        "ul", "ol", "pre", "div", "blockquote", "hr", "address", "figure", "figcaption", "form", "fieldset", "ins",
        "del", "s", "dl", "dt", "dd", "li", "table", "caption", "thead", "tfoot", "tbody", "colgroup", "col", "tr", "th",
        "td", "video", "audio", "canvas", "details", "menu", "plaintext"
    };
    StringRef inlineTags[] = {
        "object", "base", "font", "tt", "i", "b", "u", "big", "small", "em", "strong", "dfn", "code", "samp", "kbd",
        "var", "cite", "abbr", "time", "acronym", "mark", "ruby", "rt", "rp", "a", "img", "br", "wbr", "map", "q",
        "sub", "sup", "bdo", "iframe", "embed", "span", "input", "select", "textarea", "label", "button", "optgroup",
        "option", "legend", "datalist", "keygen", "output", "progress", "meter", "area", "param", "source", "track",
        "summary", "command", "device", "area", "basefont", "bgsound", "menuitem", "param", "source", "track"
    };
    StringRef emptyTags[] = {
        "meta", "link", "base", "frame", "img", "br", "wbr", "embed", "hr", "input", "keygen", "col", "command",
        "device", "area", "basefont", "bgsound", "menuitem", "param", "source", "track"
    };
    StringRef formatAsInlineTags[] = {
        "title", "a", "p", "h1", "h2", "h3", "h4", "h5", "h6", "pre", "address", "li", "th", "td", "script", "style",
        "ins", "del", "s"
    };
    StringRef preserveWhitespaceTags[] = {
        "pre", "plaintext", "title", "textarea"
        // script is not here as it is a data node, which always preserve whitespace
    };
    // todo: I think we just need submit tags, and can scrub listed
    StringRef formListedTags[] = {
        "button", "fieldset", "input", "keygen", "object", "output", "select", "textarea"
    };
    StringRef formSubmitTags[] = {
        "input", "keygen", "object", "select", "textarea"
    };
}

namespace csoup {
    void Tag::GlobalTagMap::registerTag(const csoup::StringRef &tagName, Tag* tag) {
        std::string strTagName(tagName.data(), tagName.size());
        if (globalTags_.count(strTagName)) {
            delete globalTags_[strTagName];
        }
        
        globalTags_[strTagName] = tag;
    }
    
    Tag::GlobalTagMap::GlobalTagMap() {
        for (size_t i = 0; i < arrayLength(blockTags); ++ i) {
            Tag* tag = new Tag(blockTags[i]);
            registerTag(blockTags[i], tag);
        }
        
        for (size_t i = 0; i < arrayLength(inlineTags); ++ i) {
            Tag* tag = new Tag(inlineTags[i]);
            tag->isBlock_ = false;
            tag->canContainBlock_ = false;
            tag->formatAsBlock_ = false;
            
            registerTag(inlineTags[i], tag);
        }
        
        for (size_t i = 0; i < arrayLength(emptyTags); ++ i) {
            Tag* tag = query(emptyTags[i]);
            CSOUP_ASSERT(tag != NULL);
            
            tag->canContainBlock_ = false;
            tag->canContainInline_ = false;
            tag->empty_ = true;
        }
        
        for (size_t i = 0; i < arrayLength(formatAsInlineTags); ++ i) {
            Tag* tag = query(formatAsInlineTags[i]);
            CSOUP_ASSERT(tag != NULL);
            tag->formatAsBlock_ = false;
        }
        
        for (size_t i = 0; i < arrayLength(preserveWhitespaceTags); ++ i) {
            Tag* tag = query(preserveWhitespaceTags[i]);
            CSOUP_ASSERT(tag != NULL);
            tag->preserveWhitespace_ = true;
        }
        
        for (size_t i = 0; i < arrayLength(formListedTags); ++ i) {
            Tag* tag = query(formListedTags[i]);
            CSOUP_ASSERT(tag != NULL);
            tag->formList_ = true;
        }
        
        for (size_t i = 0; i < arrayLength(formSubmitTags); ++ i) {
            Tag* tag = query(formSubmitTags[i]);
            CSOUP_ASSERT(tag != NULL);
            tag->formSubmit_ = true;
        }
    }
    
    Tag* Tag::GlobalTagMap::query(const csoup::StringRef &tagName) {
        std::string strTagName(tagName.data(), tagName.size());
        if (globalTags_.count(strTagName)) {
            return globalTags_[strTagName];
        } else {
            return NULL;
        }
    }
    
    Tag::Tag(const StringRef& tagName) :
    tagName_(tagName), isBlock_(true), formatAsBlock_(true), canContainBlock_(true), canContainInline_(true),
    empty_(false), selfClosing_(false), preserveWhitespace_(false), formList_(false), formSubmit_(false)
    {
    }
    
    bool Tag::operator==(const csoup::Tag &obj) const {
        if (this == &obj) return true;
        
        if (canContainBlock_ != obj.canContainBlock_) return false;
        if (canContainInline_ != obj.canContainInline_) return false;
        if (empty_ != obj.empty_) return false;
        if (formatAsBlock_ != obj.formatAsBlock_) return false;
        if (isBlock_ != obj.isBlock_) return false;
        if (preserveWhitespace_ != obj.preserveWhitespace_) return false;
        if (selfClosing_ != obj.selfClosing_) return false;
        if (formList_ != obj.formList_) return false;
        if (formSubmit_ != obj.formSubmit_) return false;
        
        if (!internal::strEquals(tagName_, obj.tagName_)) return false;
        
        return true;
    }
}