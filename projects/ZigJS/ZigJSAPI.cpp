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

ZigJSAPI::ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
	firingEvents = true;
	registerMethod("setImage", make_method(this, &ZigJSAPI::setImage));
    registerProperty("firingEvents",  make_property(this, &ZigJSAPI::get_firingEvents, &ZigJSAPI::set_firingEvents));
	registerMethod("update", make_method(this, &ZigJSAPI::update));
	registerAttribute("version", FBSTRING_PLUGIN_VERSION, true); 

	XN_THREAD_HANDLE handle;
	XnStatus nRetVal = xnOSCreateThread((XN_THREAD_PROC_PROTO)timerThread, this, &handle);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail start thread");
		return;
	} else {
		FBLOG_DEBUG("xnInit", "ok start thread");
	}

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

void ZigJSAPI::setUsers(const FB::VariantList& users)
{
	SetProperty("users", users);
}

void ZigJSAPI::setHands(const FB::VariantList& hands)
{
	SetProperty("hands", hands);
}

void ZigJSAPI::onHandCreate(int handId, float x, float y, float z, float time)
{
	fire_HandCreate(handId, x, y, z, time);
}

void ZigJSAPI::onHandUpdate(int handId, float x, float y, float z, float time)
{
	fire_HandUpdate(handId, x, y, z, time);
}

void ZigJSAPI::onHandDestroy(int handId, float time)
{
	fire_HandDestroy(handId, time);
}

void ZigJSAPI::onUserEntered(int userId)
{
	fire_UserEntered(userId);
}

void ZigJSAPI::onUserLeft(int userId)
{
	fire_UserLeft(userId);
}

void ZigJSAPI::onUserTrackingStarted(int userId)
{
	fire_UserTrackingStarted(userId);
}

void ZigJSAPI::onUserTrackingStopped(int userId)
{
	fire_UserTrackingStopped(userId);
}

bool ZigJSAPI::get_firingEvents()
{
	return firingEvents;
}

void ZigJSAPI::set_firingEvents(bool firingEvents)
{
	this->firingEvents = firingEvents;
}

//void ZigJSAPI::onNewFrame(const FB::variant& users, const FB::variant& hands)
void ZigJSAPI::onNewFrame(const std::string& blah)
{
	if (firingEvents) {
		//fire_NewFrame(users, hands);
		fire_NewFrame(blah);
	}
}

void ZigJSAPI::update() 
{
	getPlugin()->ReadFrame();
}

thread_ret_t XN_CALLBACK_TYPE ZigJSAPI::timerThread(void * param)
{
	ZigJSAPI * instanceRaw = (ZigJSAPI *)param;
	//TODO: should be changed to boost::dynamic_pointer_cast<shudder>() (or maybe chunder)
	ZigJSAPIWeakPtr instance = boost::dynamic_pointer_cast<ZigJSAPI>(instanceRaw->shared_from_this());
	while (1) {
		xnOSSleep(30); // TODO: something better
		ZigJSAPIPtr realPtr = instance.lock();
		if (!realPtr) return (thread_ret_t)NULL;
		realPtr->m_host->ScheduleOnMainThread(realPtr, boost::bind(&ZigJSAPI::update, realPtr));
	}
}


//TODO: unhack
void ZigJSAPI::setImage(FB::JSAPIPtr img)
{
	// unnecessary lock - we're doing everything out of the same thread now
	//boost::recursive_mutex::scoped_lock lock(m_imageMutex);
	m_image = img;
}

FB::JSAPIPtr ZigJSAPI::getImage() const
{
	// unnecessary lock - we're doing everything out of the same thread now
	//boost::recursive_mutex::scoped_lock lock(m_imageMutex);
	return m_image;
}

