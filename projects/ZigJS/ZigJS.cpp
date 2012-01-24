/**********************************************************\

  Auto-generated ZigJS.cpp

  This file contains the auto-generated main plugin object
  implementation for the awesome project

\**********************************************************/

#include "ZigJSAPI.h"

#include "ZigJS.h"
#include "json/json.h"
#include "fbjson.h"
#include "SensorOpenNI.h"

std::list< ZigJSAPIWeakPtr > ZigJS::s_listeners;
//boost::recursive_mutex ZigJS::s_listenersMutex;
SensorOpenNIPtr ZigJS::s_sensor;
FB::TimerPtr ZigJS::s_timer;

void ZigJS::AddListener(ZigJSAPIWeakPtr listener)
{
//	boost::recursive_mutex::scoped_lock lock(s_listenersMutex);
	s_listeners.push_back(listener);
}

//END JSON


void ZigJS::ReadFrame()
{
	//TODO: attempt reopening of sensor
	if (!s_sensor->ReadFrame()) {
		return; // No data, do nothing...
	}
	for(std::list<ZigJSAPIWeakPtr>::iterator i = s_listeners.begin(); i != s_listeners.end(); ) {
		ZigJSAPIPtr realPtr = i->lock();
		if (realPtr) {
			try {
				FB::JSAPIPtr image = realPtr->getImage();
				if (image) {
					image->SetProperty("src", s_sensor->GetImageBase64());
				}
				realPtr->onNewFrame(s_sensor->GetEventData());
				++i; // advance i if there were no exceptions
			} catch(const FB::script_error&) {
				// means the JSAPI is for a dead window, most likely. 
				i = s_listeners.erase(i);
			} catch(const std::runtime_error&) {
				// means the JSAPI is for a dead window, most likely. 
				i = s_listeners.erase(i);
			}
		} else {
			i = s_listeners.erase(i);
		}
	} // end of for
}


SensorOpenNIPtr ZigJS::InitSensor()
{
	return boost::make_shared<SensorOpenNI>();
}
///////////////////////////////////////////////////////////////////////////////
/// @fn ZigJS::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void ZigJS::StaticInitialize()
{
	s_sensor = InitSensor();
	s_timer = FB::Timer::getTimer(30, true, &ZigJS::ReadFrame);
	s_timer->start();
}


///////////////////////////////////////////////////////////////////////////////
/// @fn ZigJS::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void ZigJS::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
    // always be called just before the plugin library is unloaded
	if (s_timer) {
		s_timer->stop();
	}
}



///////////////////////////////////////////////////////////////////////////////
/// @brief  ZigJS constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
ZigJS::ZigJS()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  ZigJS destructor.
///////////////////////////////////////////////////////////////////////////////
ZigJS::~ZigJS()
{
    // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
    // root object) and tell the host to free the retained JSAPI objects then
    // unless you are holding another shared_ptr reference to your JSAPI object
    // they will be released here.
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void ZigJS::onPluginReady()
{
    // When this is called, the BrowserHost is attached, the JSAPI object is
    // created, and we are ready to interact with the page and such.  The
    // PluginWindow may or may not have already fire the AttachedEvent at
    // this point.
}

void ZigJS::shutdown()
{
    // This will be called when it is time for the plugin to shut down;
    // any threads or anything else that may hold a shared_ptr to this
    // object should be released here so that this object can be safely
    // destroyed. This is the last point that shared_from_this and weak_ptr
    // references to this object will be valid
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<ZigJS> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr ZigJS::createJSAPI()
{
    // m_host is the BrowserHost
    ZigJSAPIPtr newJSAPI = boost::make_shared<ZigJSAPI>(FB::ptr_cast<ZigJS>(shared_from_this()), m_host);
	ZigJS::AddListener(newJSAPI);
	return newJSAPI;
}

bool ZigJS::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool ZigJS::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool ZigJS::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool ZigJS::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
    // The window is attached; act appropriately
    return false;
}

bool ZigJS::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    // The window is about to be detached; act appropriately
    return false;
}

