/**********************************************************\

  Auto-generated ZigJSAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "ZigJSAPI.h"
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/detail/atomic.hpp>
using namespace boost::interprocess::detail;

boost::uint32_t ZigJSAPI::update_queue_count = 0;
const boost::uint32_t ZigJSAPI::max_update_queue_count = 2;


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

}

void ZigJSAPI::startTimerThread(const ZigJSAPIPtr& ptr)
{
	boost::thread t(boost::bind(&ZigJSAPI::timerThread, ptr));
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
	try {
		getPlugin()->ReadFrame();
	} catch(...) {
		atomic_dec32(&update_queue_count);
		throw;
	}
	atomic_dec32(&update_queue_count);
}

void ZigJSAPI::timerThread(ZigJSAPIPtr thisptr)
{
	//TODO: should be changed to boost::dynamic_pointer_cast<shudder>() (or maybe chunder)
	ZigJSAPIWeakPtr instance = thisptr;
	thisptr.reset();
	while (1) {
		xnOSSleep(30); // TODO: something better
		ZigJSAPIPtr realPtr = instance.lock();
		if (!realPtr) return;
		boost::uint32_t waitCount = atomic_inc32(&update_queue_count);
		if (waitCount < max_update_queue_count) {
			if (!realPtr->m_host->ScheduleOnMainThread(realPtr, boost::bind(&ZigJSAPI::update, realPtr)))
			{
				atomic_dec32(&update_queue_count);
			}
		} else {
			atomic_dec32(&update_queue_count);
		}

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

