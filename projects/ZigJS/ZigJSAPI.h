/**********************************************************\

  Auto-generated ZigJSAPI.h

\**********************************************************/

#ifndef H_ZigJSAPI
#define H_ZigJSAPI

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "ZigJS.h"

class ZigJSAPI : public FB::JSAPIAuto
{
public:
	
//	void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);

    ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~ZigJSAPI();

    ZigJSPtr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Event helpers
	FB_JSAPI_EVENT(WaveGesture, 3, (float,float,float));
	FB_JSAPI_EVENT(HandCreate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandUpdate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandDestroy, 2, (int,float));

    FB_JSAPI_EVENT(fired, 3, (const FB::variant&, bool, int));
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));
    FB_JSAPI_EVENT(notify, 0, ());

    // Method test-event
    void testEvent(const FB::variant& s);

	void setUsers(const FB::VariantList& users);
private:

	FB::variant m_users;

	ZigJSWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

	XnCallbackHandle m_gestureCB;
	XnCallbackHandle m_handCB;


    std::string m_testString;

	static void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
	static void XN_CALLBACK_TYPE HandCreateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandUpdateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandDestroyHandler(xn::HandsGenerator& generator, XnUserID user, XnFloat fTime, void* pCookie);
};

#endif // H_ZigJSAPI

