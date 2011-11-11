/**********************************************************\

  Auto-generated shitshitAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "shitshitAPI.h"
#include <boost/format.hpp>

///////////////////////////////////////////////////////////////////////////////
/// @fn shitshitAPI::shitshitAPI(const shitshitPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
shitshitAPI::shitshitAPI(const shitshitPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
    registerMethod("echo",      make_method(this, &shitshitAPI::echo));
    registerMethod("testEvent", make_method(this, &shitshitAPI::testEvent));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &shitshitAPI::get_testString,
                        &shitshitAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &shitshitAPI::get_version));


}

///////////////////////////////////////////////////////////////////////////////
/// @fn shitshitAPI::~shitshitAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
shitshitAPI::~shitshitAPI()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn shitshitPtr shitshitAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
shitshitPtr shitshitAPI::getPlugin()
{
    shitshitPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string shitshitAPI::get_testString()
{
    return m_testString;
}
void shitshitAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string shitshitAPI::get_version()
{
	shitshitPtr plugin = getPlugin();
	
    return (boost::format("%s: %d") % "test" % plugin->getLastFrameID()).str();
}

// Method echo
FB::variant shitshitAPI::echo(const FB::variant& msg)
{
    static int n(0);
    fire_echo(msg, n++);
    return msg;
}

void shitshitAPI::testEvent(const FB::variant& var)
{
    fire_fired(var, true, 1);
}

