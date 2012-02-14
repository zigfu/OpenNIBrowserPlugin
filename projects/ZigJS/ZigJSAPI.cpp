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

ZigJSAPI::ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host), m_watermark(host)
{
	firingEvents = true;
	//registerMethod("setImage", make_method(this, &ZigJSAPI::setImage));
	registerMethod("requestStreams", make_method(this, &ZigJSAPI::requestStreams));
    registerProperty("firingEvents",  make_property(this, &ZigJSAPI::get_firingEvents, &ZigJSAPI::set_firingEvents));
	registerAttribute("version", FBSTRING_PLUGIN_VERSION, true);
	//TODO: don't hardcode
	FB::VariantMap res;
	res["width"] = 160;
	res["height"] = 120;
	//TODO: fix read-only-ness (get from upstream Firebreath)
	registerAttribute("depthMapResolution", res, true);
	registerAttribute("imageMapResolution", res, true);
	registerAttribute("imageMap", "", false);
	registerAttribute("depthMap", "", false);
	registerAttribute("isZig", true, true);
	registerProperty("watermark", make_property(this, &ZigJSAPI::get_Watermark));

	registerMethod("invalidate", make_method(this, &ZigJSAPI::Invalidate));
	// only expose this when debugging (easier than using a timer by far)
	//registerMethod("test", make_method(this, &ZigJSAPI::Test));
	registerMethod("validate", make_method(this, &ZigJSAPI::Validate));

	// test every 5 secs or so
	m_watermarkTimer = FB::Timer::getTimer(5000, false, boost::bind(&ZigJSAPI::WatermarkTimerCB, this));
	m_watermarkTimer->start();
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
	m_watermarkTimer->stop();
	m_watermarkTimer.reset();
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

void ZigJSAPI::requestStreams(bool updateDepth, bool updateImage, bool isWebplayer)
{
	//note: this always happens from the main thread so it should be safe
	ZigJS::SetStreams(updateDepth, updateImage, isWebplayer);
}


//TODO: unhack
//void ZigJSAPI::setImage(FB::JSAPIPtr img)
//{
//	// unnecessary lock - we're doing everything out of the same thread now
//	//boost::recursive_mutex::scoped_lock lock(m_imageMutex);
//	m_image = img;
//}
//
//FB::JSAPIPtr ZigJSAPI::getImage() const
//{
//	// unnecessary lock - we're doing everything out of the same thread now
//	//boost::recursive_mutex::scoped_lock lock(m_imageMutex);
//	return m_image;
//}

void ZigJSAPI::WatermarkTimerCB()
{
	m_host->ScheduleOnMainThread(shared_from_this(), boost::bind(&ZigJSAPI::WatermarkTest, this));
}

void ZigJSAPI::WatermarkTest()
{
	//test only returns true if validation was successful - restart the timer in that case
	if (Test()) {
		m_watermarkTimer->start();
	}
}

