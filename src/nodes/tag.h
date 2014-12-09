#ifndef CSOUP_TAG_H_
#define CSOUP_TAG_H_

#include "../util/common.h"

namespace csoup {
    class StringRef;
    
    /**
     * An enum for all the tags defined in the HTML5 standard.  These correspond to
     * the tag names themselves.  Enum constants exist only for tags which appear in
     * the spec itself (or for tags with special handling in the SVG and MathML
     * namespaces); any other tags appear as CSOUP_TAG_UNKNOWN and the actual tag
     * name can be obtained through original_tag.
     *
     * This is mostly for API convenience, so that clients of this library don't
     * need to perform a strcasecmp to find the normalized tag name.  It also has
     * efficiency benefits, by letting the parser work with enums instead of
     * strings.
     */
    typedef enum {
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/semantics.html#the-root-element
        CSOUP_TAG_HTML,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/semantics.html#document-metadata
        CSOUP_TAG_HEAD,
        CSOUP_TAG_TITLE,
        CSOUP_TAG_BASE,
        CSOUP_TAG_LINK,
        CSOUP_TAG_META,
        CSOUP_TAG_STYLE,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/scripting-1.html#scripting-1
        CSOUP_TAG_SCRIPT,
        CSOUP_TAG_NOSCRIPT,
        CSOUP_TAG_TEMPLATE,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/sections.html#sections
        CSOUP_TAG_BODY,
        CSOUP_TAG_ARTICLE,
        CSOUP_TAG_SECTION,
        CSOUP_TAG_NAV,
        CSOUP_TAG_ASIDE,
        CSOUP_TAG_H1,
        CSOUP_TAG_H2,
        CSOUP_TAG_H3,
        CSOUP_TAG_H4,
        CSOUP_TAG_H5,
        CSOUP_TAG_H6,
        CSOUP_TAG_HGROUP,
        CSOUP_TAG_HEADER,
        CSOUP_TAG_FOOTER,
        CSOUP_TAG_ADDRESS,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/grouping-content.html#grouping-content
        CSOUP_TAG_P,
        CSOUP_TAG_HR,
        CSOUP_TAG_PRE,
        CSOUP_TAG_BLOCKQUOTE,
        CSOUP_TAG_OL,
        CSOUP_TAG_UL,
        CSOUP_TAG_LI,
        CSOUP_TAG_DL,
        CSOUP_TAG_DT,
        CSOUP_TAG_DD,
        CSOUP_TAG_FIGURE,
        CSOUP_TAG_FIGCAPTION,
        CSOUP_TAG_MAIN,
        CSOUP_TAG_DIV,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/text-level-semantics.html#text-level-semantics
        CSOUP_TAG_A,
        CSOUP_TAG_EM,
        CSOUP_TAG_STRONG,
        CSOUP_TAG_SMALL,
        CSOUP_TAG_S,
        CSOUP_TAG_CITE,
        CSOUP_TAG_Q,
        CSOUP_TAG_DFN,
        CSOUP_TAG_ABBR,
        CSOUP_TAG_DATA,
        CSOUP_TAG_TIME,
        CSOUP_TAG_CODE,
        CSOUP_TAG_VAR,
        CSOUP_TAG_SAMP,
        CSOUP_TAG_KBD,
        CSOUP_TAG_SUB,
        CSOUP_TAG_SUP,
        CSOUP_TAG_I,
        CSOUP_TAG_B,
        CSOUP_TAG_U,
        CSOUP_TAG_MARK,
        CSOUP_TAG_RUBY,
        CSOUP_TAG_RT,
        CSOUP_TAG_RP,
        CSOUP_TAG_BDI,
        CSOUP_TAG_BDO,
        CSOUP_TAG_SPAN,
        CSOUP_TAG_BR,
        CSOUP_TAG_WBR,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/edits.html#edits
        CSOUP_TAG_INS,
        CSOUP_TAG_DEL,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/embedded-content-1.html#embedded-content-1
        CSOUP_TAG_IMAGE,
        CSOUP_TAG_IMG,
        CSOUP_TAG_IFRAME,
        CSOUP_TAG_EMBED,
        CSOUP_TAG_OBJECT,
        CSOUP_TAG_PARAM,
        CSOUP_TAG_VIDEO,
        CSOUP_TAG_AUDIO,
        CSOUP_TAG_SOURCE,
        CSOUP_TAG_TRACK,
        CSOUP_TAG_CANVAS,
        CSOUP_TAG_MAP,
        CSOUP_TAG_AREA,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-map-element.html#mathml
        CSOUP_TAG_MATH,
        CSOUP_TAG_MI,
        CSOUP_TAG_MO,
        CSOUP_TAG_MN,
        CSOUP_TAG_MS,
        CSOUP_TAG_MTEXT,
        CSOUP_TAG_MGLYPH,
        CSOUP_TAG_MALIGNMARK,
        CSOUP_TAG_ANNOTATION_XML,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-map-element.html#svg-0
        CSOUP_TAG_SVG,
        CSOUP_TAG_FOREIGNOBJECT,
        CSOUP_TAG_DESC,
        // SVG title tags will have CSOUP_TAG_TITLE as with HTML.
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/tabular-data.html#tabular-data
        CSOUP_TAG_TABLE,
        CSOUP_TAG_CAPTION,
        CSOUP_TAG_COLGROUP,
        CSOUP_TAG_COL,
        CSOUP_TAG_TBODY,
        CSOUP_TAG_THEAD,
        CSOUP_TAG_TFOOT,
        CSOUP_TAG_TR,
        CSOUP_TAG_TD,
        CSOUP_TAG_TH,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/forms.html#forms
        CSOUP_TAG_FORM,
        CSOUP_TAG_FIELDSET,
        CSOUP_TAG_LEGEND,
        CSOUP_TAG_LABEL,
        CSOUP_TAG_INPUT,
        CSOUP_TAG_BUTTON,
        CSOUP_TAG_SELECT,
        CSOUP_TAG_DATALIST,
        CSOUP_TAG_OPTGROUP,
        CSOUP_TAG_OPTION,
        CSOUP_TAG_TEXTAREA,
        CSOUP_TAG_KEYGEN,
        CSOUP_TAG_OUTPUT,
        CSOUP_TAG_PROGRESS,
        CSOUP_TAG_METER,
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/interactive-elements.html#interactive-elements
        CSOUP_TAG_DETAILS,
        CSOUP_TAG_SUMMARY,
        CSOUP_TAG_MENU,
        CSOUP_TAG_MENUITEM,
        // Non-conforming elements that nonetheless appear in the HTML5 spec.
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/obsolete.html#non-conforming-features
        CSOUP_TAG_APPLET,
        CSOUP_TAG_ACRONYM,
        CSOUP_TAG_BGSOUND,
        CSOUP_TAG_DIR,
        CSOUP_TAG_FRAME,
        CSOUP_TAG_FRAMESET,
        CSOUP_TAG_NOFRAMES,
        CSOUP_TAG_ISINDEX,
        CSOUP_TAG_LISTING,
        CSOUP_TAG_XMP,
        CSOUP_TAG_NEXTID,
        CSOUP_TAG_NOEMBED,
        CSOUP_TAG_PLAINTEXT,
        CSOUP_TAG_RB,
        CSOUP_TAG_STRIKE,
        CSOUP_TAG_BASEFONT,
        CSOUP_TAG_BIG,
        CSOUP_TAG_BLINK,
        CSOUP_TAG_CENTER,
        CSOUP_TAG_FONT,
        CSOUP_TAG_MARQUEE,
        CSOUP_TAG_MULTICOL,
        CSOUP_TAG_NOBR,
        CSOUP_TAG_SPACER,
        CSOUP_TAG_TT,
        // Used for all tags that don't have special handling in HTML.
        CSOUP_TAG_UNKNOWN,
        // A marker value to indicate the end of the enum, for iterating over it.
        // Also used as the terminator for varargs functions that take tags.
        CSOUP_TAG_LAST,
    } TagEnum;
    
    typedef enum {
        CSOUP_NAMESPACE_HTML,
        CSOUP_NAMESPACE_SVG,
        CSOUP_NAMESPACE_MATHML
    } TagNamespaceEnum;
    
    extern StringRef tagEnumToName(TagEnum tag);
    extern TagEnum tagEnumFromText(const StringRef& name);
    extern TagEnum tagEnumFromText(const CharType* name);
}

#endif