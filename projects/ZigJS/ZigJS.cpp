/**********************************************************\

  Auto-generated ZigJS.cpp

  This file contains the auto-generated main plugin object
  implementation for the awesome project

\**********************************************************/

#include "ZigJSAPI.h"

#include "ZigJS.h"
#include "json/json.h"
#include "fbjson.h"
#include "Sensor.h"
#include <cstdlib> // TODO: this is for srand(), do some real random number generation
std::list< ZigJSAPIWeakPtr > ZigJS::s_listeners;
//boost::recursive_mutex ZigJS::s_listenersMutex;
SensorPtr ZigJS::s_sensor;
FB::TimerPtr ZigJS::s_timer;

// TODO: getting lazy with the class-belongingness here
boost::mutex s_hostsMutex;
std::list< FB::BrowserHostWeakPtr > s_hosts;

void ZigJS::AddListener(ZigJSAPIWeakPtr listener)
{
//	boost::recursive_mutex::scoped_lock lock(s_listenersMutex);
	s_listeners.push_back(listener);
}
//TODO: something that handles multiple tabs in a sane fashion
bool ZigJS::s_getImage = false;
bool ZigJS::s_getDepth = false;
bool ZigJS::s_isWebplayer = false;
void ZigJS::SetStreams(bool getDepth, bool getImage, bool isWebplayer)
{
	s_getImage = getImage;
	s_getDepth = getDepth;
	s_isWebplayer = isWebplayer;
}
const int REOPEN_WAIT_FRAMES = 450;
void ZigJS::ReadFrame(void *)
{
	//TODO: another hack for raising the event.
	// All this code smells and needs to be refactored - the wonders of 
	// releasing quickly :(
	static bool wasSensorConnected = false;
	bool isSensorOk = true;
	bool newDataAvailable = false;
	// try reopening the sensor
	if (!s_sensor) {
		isSensorOk = false;
		//TODO: mega-hack :(
		static int frameRestCount = REOPEN_WAIT_FRAMES;
		//s_sensor.reset();
		frameRestCount++;
		if (frameRestCount > REOPEN_WAIT_FRAMES) { //~10sec wait - magic number needed by OpenNI :(
									//TODO: find another way
			s_sensor = InitSensor();
			frameRestCount = 0;
			if (s_sensor) isSensorOk = true;
		} else {
			return;
		}
	}
	if (isSensorOk && !s_sensor->Valid()) {
		s_sensor.reset();
		isSensorOk = false;
	}

	if (isSensorOk && s_sensor->ReadFrame(s_getDepth, s_getImage, s_isWebplayer)) {
		// New data!!!
		newDataAvailable = true;
	}

	for(std::list<ZigJSAPIWeakPtr>::iterator i = s_listeners.begin(); i != s_listeners.end(); ) {
		ZigJSAPIPtr realPtr = i->lock();
		if (realPtr) {

			try {
				//FB::JSAPIPtr image = realPtr->getImage();
				//if (image) {
				//	image->SetProperty("src", *(s_sensor->GetImageBase64()));
				//}
				if (isSensorOk != wasSensorConnected) {
					realPtr->onStatusChange(isSensorOk);
				}
				if (isSensorOk && newDataAvailable) {
					if (s_getImage) {
						//realPtr->unregisterAttribute("imageMap");
						realPtr->setImageMap(s_sensor->GetImage());
					}
					if (s_getDepth) { 
						//realPtr->unregisterAttribute("depthMap");
						realPtr->setDepthMap(s_sensor->GetDepth());
					}
					realPtr->onNewFrame(s_sensor->GetEventData());
				}
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
	wasSensorConnected = isSensorOk;
}


SensorPtr ZigJS::InitSensor()
{
	return Sensor::CreateSensor();
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
	srand((unsigned)time(0));
	s_sensor = InitSensor();
	s_timer = FB::Timer::getTimer(30, true, &ZigJS::TimerCallback);
	s_timer->start();
}

void ZigJS::TimerCallback()
{
	FB::BrowserHostPtr host;
	{
		boost::mutex::scoped_lock lock(s_hostsMutex);
		for(std::list<FB::BrowserHostWeakPtr>::iterator i = s_hosts.begin(); i != s_hosts.end(); ) {
			FB::BrowserHostPtr realPtr = i->lock();
			if (realPtr) {
				host = realPtr;
				++i; // keep going to make sure we clean stale instances
			} else {
				i = s_hosts.erase(i);
			}
		}
	}
	if (host) {
		host->ScheduleAsyncCall(&ZigJS::ReadFrame, NULL);
	}
}

bool ZigJS::IsSensorConnected()
{
	return (s_sensor) && (s_sensor->Valid());
}
FB::VariantList ZigJS::ConvertImageToWorldSpace(const std::vector<double>& points)
{
	if (s_sensor) {
		return s_sensor->convertImageToWorldSpace(points);
	}
	//TODO: throw some exception instead?
	return FB::VariantList();
}
FB::VariantList ZigJS::ConvertWorldToImageSpace(const std::vector<double>& points)
{
	if (s_sensor) {
		return s_sensor->convertWorldToImageSpace(points);
	}
	//TODO: throw some exception instead?
	return FB::VariantList();
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
		s_timer.reset();
	}
	s_sensor.reset();
	Sensor::Unload();
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
	boost::mutex::scoped_lock lock(s_hostsMutex);
	s_hosts.push_front(m_host);
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

