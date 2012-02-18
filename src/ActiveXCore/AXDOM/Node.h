/**********************************************************\
Original Author: Richard Bateman (taxilian)

Created:    Sep 21, 2010
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2010 Facebook, Inc and the Firebreath development team
\**********************************************************/

#pragma once
#ifndef H_AXDOM_NODE
#define H_AXDOM_NODE

#include <string>
#include "win_common.h"
#include <atlctl.h>
#include "IDispatchAPI.h"
#include "JSObject.h"
#include "DOM/Node.h"

namespace FB { namespace ActiveX {
    namespace AXDOM {
        class Node;
        typedef boost::shared_ptr<Node> NodePtr;

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @class  Node
        ///
        /// @brief  Provides an ActiveX specific implementation of DOM::Node
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        class Node : public virtual FB::DOM::Node
        {
        public:
            Node(const FB::JSObjectPtr& element, IWebBrowser *web)
                : m_axNode(FB::ptr_cast<IDispatchAPI>(element)->getIDispatch()),
                  m_webBrowser(web), FB::DOM::Node(element)
            {
            }
            virtual ~Node() { }

        public:
			virtual void appendChild(FB::DOM::NodePtr node) {
				CComPtr<IHTMLDOMNode> newNode;
				AXDOM::Node* actualNode = dynamic_cast<AXDOM::Node*>(&(*node));
				if (SUCCEEDED(m_axNode->appendChild(actualNode->m_axNode, &newNode))) {
					actualNode->m_axNode = newNode; // set the node to point to the real IHTMLDOMNode
				} else {
					throw std::runtime_error("failed to appendChild!");
				}
			}
        protected:
            CComQIPtr<IHTMLDOMNode> m_axNode;
            CComPtr<IWebBrowser> m_webBrowser;
        };

    };
} }
#endif // H_AXDOM_NODE

