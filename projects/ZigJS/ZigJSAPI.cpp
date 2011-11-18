/**********************************************************\

  Auto-generated ZigJSAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "ZigJSAPI.h"
#include <boost/format.hpp>



///////////////////////////////////////////////////////////////////////////////
/// @fn ZigJSAPI::ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////


void XN_CALLBACK_TYPE ZigJSAPI::GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie)
{
	((ZigJSAPI*)pCookie)->fire_WaveGesture(pEndPosition->X,pEndPosition->Y,pEndPosition->Z);
	ZigJS::s_hands.StartTracking(*pEndPosition);
}

void XN_CALLBACK_TYPE ZigJSAPI::HandCreateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
{
	((ZigJSAPI*)pCookie)->fire_HandCreate((int)user,pPosition->X,pPosition->Y,pPosition->Z, (float)fTime);
}

void XN_CALLBACK_TYPE ZigJSAPI::HandUpdateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
{
	((ZigJSAPI*)pCookie)->fire_HandUpdate((int)user,pPosition->X,pPosition->Y,pPosition->Z, (float)fTime);
}

void XN_CALLBACK_TYPE ZigJSAPI::HandDestroyHandler(xn::HandsGenerator& generator, XnUserID user, XnFloat fTime, void* pCookie)
{
	((ZigJSAPI*)pCookie)->fire_HandDestroy((int)user, (float)fTime);
}



ZigJSAPI::ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
    registerMethod("echo",      make_method(this, &ZigJSAPI::echo));
    registerMethod("testEvent", make_method(this, &ZigJSAPI::testEvent));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &ZigJSAPI::get_testString,
                        &ZigJSAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &ZigJSAPI::get_version));

	
	ZigJS::s_gestures.RegisterGestureCallbacks(&ZigJSAPI::GestureRecognizedHandler, NULL, this, m_gestureCB);
	ZigJS::s_hands.RegisterHandCallbacks(&ZigJSAPI::HandCreateHandler, &ZigJSAPI::HandUpdateHandler, &ZigJSAPI::HandDestroyHandler, this, m_handCB);
}



///////////////////////////////////////////////////////////////////////////////
/// @fn ZigJSAPI::~ZigJSAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
ZigJSAPI::~ZigJSAPI()
{
	ZigJS::s_gestures.UnregisterGestureCallbacks(m_gestureCB);
	ZigJS::s_hands.UnregisterHandCallbacks(m_handCB);
}


///////////////////////////////////////////////////////////////////////////////
/// @fn ZigJSPtr ZigJSAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
ZigJSPtr ZigJSAPI::getPlugin()
{
    ZigJSPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string ZigJSAPI::get_testString()
{
    return m_testString;
}
void ZigJSAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string ZigJSAPI::get_version()
{
    return (boost::format("%s: %d") % "test" % ZigJS::s_lastFrame).str();
}

// Method echo
FB::variant ZigJSAPI::echo(const FB::variant& msg)
{
    static int n(0);
    fire_echo(msg, n++);
    return msg;
}

void ZigJSAPI::testEvent(const FB::variant& var)
{
    fire_fired(var, true, 1);
}

void ZigJSAPI::setUsers(const FB::VariantList& users)
{
	SetProperty("users", users);
}
