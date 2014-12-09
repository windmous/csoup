#include "tag.h"
#include "stringref.h"

namespace {
// if you changed your
#define STRINGREF_INITIALIZER(str) {str, sizeof(str) / sizeof(*str) - 1}
    
    const csoup::StringRef tagNames[] = {
        STRINGREF_INITIALIZER("html"),
        STRINGREF_INITIALIZER("head"),
        STRINGREF_INITIALIZER("title"),
        STRINGREF_INITIALIZER("base"),
        STRINGREF_INITIALIZER("link"),
        STRINGREF_INITIALIZER("meta"),
        STRINGREF_INITIALIZER("style"),
        STRINGREF_INITIALIZER("script"),
        STRINGREF_INITIALIZER("noscript"),
        STRINGREF_INITIALIZER("template"),
        STRINGREF_INITIALIZER("body"),
        STRINGREF_INITIALIZER("article"),
        STRINGREF_INITIALIZER("section"),
        STRINGREF_INITIALIZER("nav"),
        STRINGREF_INITIALIZER("aside"),
        STRINGREF_INITIALIZER("h1"),
        STRINGREF_INITIALIZER("h2"),
        STRINGREF_INITIALIZER("h3"),
        STRINGREF_INITIALIZER("h4"),
        STRINGREF_INITIALIZER("h5"),
        STRINGREF_INITIALIZER("h6"),
        STRINGREF_INITIALIZER("hgroup"),
        STRINGREF_INITIALIZER("header"),
        STRINGREF_INITIALIZER("footer"),
        STRINGREF_INITIALIZER("address"),
        STRINGREF_INITIALIZER("p"),
        STRINGREF_INITIALIZER("hr"),
        STRINGREF_INITIALIZER("pre"),
        STRINGREF_INITIALIZER("blockquote"),
        STRINGREF_INITIALIZER("ol"),
        STRINGREF_INITIALIZER("ul"),
        STRINGREF_INITIALIZER("li"),
        STRINGREF_INITIALIZER("dl"),
        STRINGREF_INITIALIZER("dt"),
        STRINGREF_INITIALIZER("dd"),
        STRINGREF_INITIALIZER("figure"),
        STRINGREF_INITIALIZER("figcaption"),
        STRINGREF_INITIALIZER("main"),
        STRINGREF_INITIALIZER("div"),
        STRINGREF_INITIALIZER("a"),
        STRINGREF_INITIALIZER("em"),
        STRINGREF_INITIALIZER("strong"),
        STRINGREF_INITIALIZER("small"),
        STRINGREF_INITIALIZER("s"),
        STRINGREF_INITIALIZER("cite"),
        STRINGREF_INITIALIZER("q"),
        STRINGREF_INITIALIZER("dfn"),
        STRINGREF_INITIALIZER("abbr"),
        STRINGREF_INITIALIZER("data"),
        STRINGREF_INITIALIZER("time"),
        STRINGREF_INITIALIZER("code"),
        STRINGREF_INITIALIZER("var"),
        STRINGREF_INITIALIZER("samp"),
        STRINGREF_INITIALIZER("kbd"),
        STRINGREF_INITIALIZER("sub"),
        STRINGREF_INITIALIZER("sup"),
        STRINGREF_INITIALIZER("i"),
        STRINGREF_INITIALIZER("b"),
        STRINGREF_INITIALIZER("u"),
        STRINGREF_INITIALIZER("mark"),
        STRINGREF_INITIALIZER("ruby"),
        STRINGREF_INITIALIZER("rt"),
        STRINGREF_INITIALIZER("rp"),
        STRINGREF_INITIALIZER("bdi"),
        STRINGREF_INITIALIZER("bdo"),
        STRINGREF_INITIALIZER("span"),
        STRINGREF_INITIALIZER("br"),
        STRINGREF_INITIALIZER("wbr"),
        STRINGREF_INITIALIZER("ins"),
        STRINGREF_INITIALIZER("del"),
        STRINGREF_INITIALIZER("image"),
        STRINGREF_INITIALIZER("img"),
        STRINGREF_INITIALIZER("iframe"),
        STRINGREF_INITIALIZER("embed"),
        STRINGREF_INITIALIZER("object"),
        STRINGREF_INITIALIZER("param"),
        STRINGREF_INITIALIZER("video"),
        STRINGREF_INITIALIZER("audio"),
        STRINGREF_INITIALIZER("source"),
        STRINGREF_INITIALIZER("track"),
        STRINGREF_INITIALIZER("canvas"),
        STRINGREF_INITIALIZER("map"),
        STRINGREF_INITIALIZER("area"),
        STRINGREF_INITIALIZER("math"),
        STRINGREF_INITIALIZER("mi"),
        STRINGREF_INITIALIZER("mo"),
        STRINGREF_INITIALIZER("mn"),
        STRINGREF_INITIALIZER("ms"),
        STRINGREF_INITIALIZER("mtext"),
        STRINGREF_INITIALIZER("mglyph"),
        STRINGREF_INITIALIZER("malignmark"),
        STRINGREF_INITIALIZER("annotation-xml"),
        STRINGREF_INITIALIZER("svg"),
        STRINGREF_INITIALIZER("foreignobject"),
        STRINGREF_INITIALIZER("desc"),
        STRINGREF_INITIALIZER("table"),
        STRINGREF_INITIALIZER("caption"),
        STRINGREF_INITIALIZER("colgroup"),
        STRINGREF_INITIALIZER("col"),
        STRINGREF_INITIALIZER("tbody"),
        STRINGREF_INITIALIZER("thead"),
        STRINGREF_INITIALIZER("tfoot"),
        STRINGREF_INITIALIZER("tr"),
        STRINGREF_INITIALIZER("td"),
        STRINGREF_INITIALIZER("th"),
        STRINGREF_INITIALIZER("form"),
        STRINGREF_INITIALIZER("fieldset"),
        STRINGREF_INITIALIZER("legend"),
        STRINGREF_INITIALIZER("label"),
        STRINGREF_INITIALIZER("input"),
        STRINGREF_INITIALIZER("button"),
        STRINGREF_INITIALIZER("select"),
        STRINGREF_INITIALIZER("datalist"),
        STRINGREF_INITIALIZER("optgroup"),
        STRINGREF_INITIALIZER("option"),
        STRINGREF_INITIALIZER("textarea"),
        STRINGREF_INITIALIZER("keygen"),
        STRINGREF_INITIALIZER("output"),
        STRINGREF_INITIALIZER("progress"),
        STRINGREF_INITIALIZER("meter"),
        STRINGREF_INITIALIZER("details"),
        STRINGREF_INITIALIZER("summary"),
        STRINGREF_INITIALIZER("menu"),
        STRINGREF_INITIALIZER("menuitem"),
        STRINGREF_INITIALIZER("applet"),
        STRINGREF_INITIALIZER("acronym"),
        STRINGREF_INITIALIZER("bgsound"),
        STRINGREF_INITIALIZER("dir"),
        STRINGREF_INITIALIZER("frame"),
        STRINGREF_INITIALIZER("frameset"),
        STRINGREF_INITIALIZER("noframes"),
        STRINGREF_INITIALIZER("isindex"),
        STRINGREF_INITIALIZER("listing"),
        STRINGREF_INITIALIZER("xmp"),
        STRINGREF_INITIALIZER("nextid"),
        STRINGREF_INITIALIZER("noembed"),
        STRINGREF_INITIALIZER("plaintext"),
        STRINGREF_INITIALIZER("rb"),
        STRINGREF_INITIALIZER("strike"),
        STRINGREF_INITIALIZER("basefont"),
        STRINGREF_INITIALIZER("big"),
        STRINGREF_INITIALIZER("blink"),
        STRINGREF_INITIALIZER("center"),
        STRINGREF_INITIALIZER("font"),
        STRINGREF_INITIALIZER("marquee"),
        STRINGREF_INITIALIZER("multicol"),
        STRINGREF_INITIALIZER("nobr"),
        STRINGREF_INITIALIZER("spacer"),
        STRINGREF_INITIALIZER("tt"),
        STRINGREF_INITIALIZER(""),   // TAG_UNKNOW)N
        STRINGREF_INITIALIZER(""),   // TAG_LAST
    };
#undef STRINGREF_INITIALIZER
    
    CSOUP_STATIC_ASSERT(csoup::CSOUP_TAG_LAST == (sizeof(tagNames) / sizeof(*tagNames) - 1));
}

namespace csoup {
    StringRef tagEnumToName(TagEnum tag) {
        CSOUP_ASSERT(tag <= CSOUP_TAG_LAST);
        return tagNames[tag];
    }
    
    TagEnum tagEnumFromText(const StringRef& name) {
        if (name.data() == NULL || name.size() == 0) {
            return CSOUP_TAG_UNKNOWN;
        }
        
        for (int i = 0; i <= CSOUP_TAG_LAST; ++ i) {
            if (internal::strEqualsIgnoreCase(name, tagNames[i])) {
                return static_cast<TagEnum>(i);
            }
        }
        
        return CSOUP_TAG_UNKNOWN;
    }
    
    TagEnum tagEnumFromText(const CharType* name) {
        return tagEnumFromText(StringRef(name));
    }

}