/*
 *  This file is part of the WebKit open source project.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#if !defined(__WEBKITDOM_H_INSIDE__) && !defined(BUILDING_WEBKIT)
#error "Only <webkitdom/webkitdom.h> can be included directly."
#endif

#ifndef WebKitDOMHTMLMenuElement_h
#define WebKitDOMHTMLMenuElement_h

#include <glib-object.h>
#include <webkitdom/WebKitDOMHTMLElement.h>
#include <webkitdom/webkitdomdefines.h>

G_BEGIN_DECLS

#define WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT            (webkit_dom_html_menu_element_get_type())
#define WEBKIT_DOM_HTML_MENU_ELEMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT, WebKitDOMHTMLMenuElement))
#define WEBKIT_DOM_HTML_MENU_ELEMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT, WebKitDOMHTMLMenuElementClass)
#define WEBKIT_DOM_IS_HTML_MENU_ELEMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT))
#define WEBKIT_DOM_IS_HTML_MENU_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT))
#define WEBKIT_DOM_HTML_MENU_ELEMENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  WEBKIT_DOM_TYPE_HTML_MENU_ELEMENT, WebKitDOMHTMLMenuElementClass))

struct _WebKitDOMHTMLMenuElement {
    WebKitDOMHTMLElement parent_instance;
};

struct _WebKitDOMHTMLMenuElementClass {
    WebKitDOMHTMLElementClass parent_class;
};

WEBKIT_API GType
webkit_dom_html_menu_element_get_type(void);

/**
 * webkit_dom_html_menu_element_get_compact:
 * @self: A #WebKitDOMHTMLMenuElement
 *
 * Returns: A #gboolean
**/
WEBKIT_API gboolean
webkit_dom_html_menu_element_get_compact(WebKitDOMHTMLMenuElement* self);

/**
 * webkit_dom_html_menu_element_set_compact:
 * @self: A #WebKitDOMHTMLMenuElement
 * @value: A #gboolean
 *
**/
WEBKIT_API void
webkit_dom_html_menu_element_set_compact(WebKitDOMHTMLMenuElement* self, gboolean value);

G_END_DECLS

#endif /* WebKitDOMHTMLMenuElement_h */